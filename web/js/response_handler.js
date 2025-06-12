// web/js/response_handler.js
import { displayResult } from './ui.js';

export function handleBackendResponse(response, commandName) {
    const activeModule = document.getElementById(commandName); // 使用传入的 commandName
    if (!activeModule) {
        console.warn(`No active module found for command: ${commandName}. Response: ${response}`);
        // Fallback to global status or general message
        const globalStatusDiv = document.getElementById('globalStatus');
        if (globalStatusDiv) {
            globalStatusDiv.textContent = `Server responded: ${response}`;
            globalStatusDiv.classList.remove('connected', 'disconnected');
            globalStatusDiv.classList.add('info');
        }
        return;
    }

    const resultP = activeModule.querySelector('.result-area p');
    const detailDiv = activeModule.querySelector('.result-area .result-details, .result-area div[id$="Details"]'); // 查找复杂结果的显示区域

    // 清空详细结果区域
    if (detailDiv) detailDiv.innerHTML = '';

    // 通用错误处理
    if (response === '-1') {
        displayResult(resultP, '操作失败: -1', 'failure');
    }
    // 通用成功处理
    else if (response === '0') {
        displayResult(resultP, '操作成功: 0', 'success');
    }
    // buy_ticket 的特殊成功返回 'queue'
    else if (response === 'queue' && commandName === 'buy_ticket') {
        displayResult(resultP, '购买成功: queue (已加入候补队列)', 'info');
    }
    // exit 命令的特殊返回 'bye'
    else if (commandName === 'exit_backend' && response === 'bye') { // 注意这里的 commandName 仍然是 'exit_backend'，因为是模块ID
        displayResult(resultP, `退出后端成功: ${response}`, 'success');
        // 注意：此时 WebSocket 会断开并尝试重连，`onclose` 会处理
    }
    else {
        // 根据命令名解析和显示复杂结果
        switch (commandName) {
            case 'query_profile':
            case 'modify_profile':
                // <username> <name> <mailAddr> <privilege>
                const profileParts = response.split(' ');
                if (profileParts.length === 4) {
                    displayResult(resultP, `
                        <p>用户名: <span class="result-text-highlight-1">${profileParts[0]}</span></p>
                        <p>真实姓名: <span class="result-text-highlight-2">${profileParts[1]}</span></p>
                        <p>邮箱: <span class="result-text-highlight-4">${profileParts[2]}</span></p>
                        <p>权限: <span class="result-text-highlight-3">${profileParts[3]}</span></p>
                    `, 'success');
                } else {
                    displayResult(resultP, `非预期结果格式: ${response}`, 'failure');
                }
                break;

            case 'query_train':
                // 第一行为 <trainID> <type>
                // 接下来 <stationNum> 行，每行为 <stations[i]> <ARRIVING_TIME> -> <LEAVING_TIME> <PRICE> <SEAT>
                const trainLines = response.split('\n').filter(line => line.trim() !== '');
                if (trainLines.length >= 2) {
                    const [trainIdType, ...stationDetails] = trainLines;
                    displayResult(resultP, `列车信息: <span class="result-text-highlight-1">${trainIdType.split(' ')[0]}</span> (类型: <span class="result-text-highlight-4">${trainIdType.split(' ')[1]}</span>)`, 'success');

                    if (detailDiv) {
                        let tableHtml = '<table class="result-table"><thead><tr><th>车站</th><th>到达时间</th><th>离开时间</th><th>累计票价</th><th>到下一站余票</th></tr></thead><tbody>';
                        stationDetails.forEach(line => {
                            const parts = line.split(' ');
                            const stationName = parts[0];
                            const arrivingTime = `${parts[1]} ${parts[2]}`; // HH:MM
                            const leavingTime = `${parts[4]} ${parts[5]}`; // HH:MM
                            const price = parts[6];
                            const seat = parts[7];
                            tableHtml += `
                                <tr>
                                    <td><span class="result-text-highlight-2">${stationName}</span></td>
                                    <td>${arrivingTime}</td>
                                    <td>${leavingTime}</td>
                                    <td><span class="result-text-highlight-3">${price}</span></td>
                                    <td><span class="result-text-highlight-3">${seat}</span></td>
                                </tr>
                            `;
                        });
                        tableHtml += '</tbody></table>';
                        detailDiv.innerHTML = tableHtml;
                    }
                } else {
                    displayResult(resultP, `查询失败或无数据: ${response}`, 'failure');
                }
                break;

            case 'query_ticket':
            case 'query_transfer':
                // query_ticket: 第一行整数数量，接下来每行 <trainID> <FROM> <LEAVING_TIME> -> <TO> <ARRIVING_TIME> <PRICE> <SEAT>
                // query_transfer: 0 (失败) 或 2行车次信息
                const ticketLines = response.split('\n').filter(line => line.trim() !== '');
                const count = parseInt(ticketLines[0]);

                if (isNaN(count) || count < 0) {
                    displayResult(resultP, `非预期结果格式或查询失败: ${response}`, 'failure');
                    break;
                }

                if (count === 0 && commandName === 'query_transfer') {
                    displayResult(resultP, '查询中转失败: 没有符合要求的车次 (0)', 'failure');
                } else if (count === 0 && commandName === 'query_ticket') {
                    displayResult(resultP, '查询余票成功: 没有符合要求的车次 (0)', 'info');
                }
                else {
                    displayResult(resultP, `${commandName === 'query_ticket' ? '查询到余票' : '查询到中转方案'}: 共 <span class="result-text-highlight-3">${count}</span> 条`, 'success');
                    if (detailDiv) {
                        let tableHtml = '<table class="result-table"><thead><tr><th>车次ID</th><th>出发站</th><th>出发时间</th><th>到达站</th><th>到达时间</th><th>价格</th><th>余票/可购</th></tr></thead><tbody>';
                        for (let i = 1; i < ticketLines.length; i++) {
                            const line = ticketLines[i];
                            const parts = line.split(' ');
                            // 根据实际解析结果调整索引
                            const trainId = parts[0];
                            const fromStation = parts[1];
                            const leavingTime = `${parts[2]} ${parts[3]}`; // 日期 时间
                            const toStation = parts[5];
                            const arrivingTime = `${parts[6]} ${parts[7]}`; // 日期 时间
                            const price = parts[8];
                            const seat = parts[9];

                            tableHtml += `
                                <tr>
                                    <td><span class="result-text-highlight-1">${trainId}</span></td>
                                    <td><span class="result-text-highlight-5">${fromStation}</span></td>
                                    <td><span class="result-text-highlight-2">${leavingTime}</span></td>
                                    <td><span class="result-text-highlight-5">${toStation}</span></td>
                                    <td><span class="result-text-highlight-2">${arrivingTime}</span></td>
                                    <td><span class="result-text-highlight-3">${price}</span></td>
                                    <td><span class="result-text-highlight-3">${seat}</span></td>
                                </tr>
                            `;
                        }
                        tableHtml += '</tbody></table>';
                        detailDiv.innerHTML = tableHtml;
                    }
                }
                break;

            case 'buy_ticket':
                // 购买成功返回整数总价
                const totalPrice = parseInt(response);
                if (!isNaN(totalPrice) && totalPrice >= 0) {
                    displayResult(resultP, `购买成功！订单总价: <span class="result-text-highlight-3">${totalPrice}</span>`, 'success');
                } else {
                    displayResult(resultP, `购票失败或非预期结果: ${response}`, 'failure');
                }
                break;

            case 'query_order':
                // 第一行整数订单数量，接下来每行 [<STATUS>] <trainID> <FROM> <LEAVING_TIME> -> <TO> <ARRIVING_TIME> <PRICE> <NUM>
                const orderLines = response.split('\n').filter(line => line.trim() !== '');
                const orderCount = parseInt(orderLines[0]);

                if (isNaN(orderCount) || orderCount < 0) {
                    displayResult(resultP, `非预期结果格式或查询失败: ${response}`, 'failure');
                    break;
                }

                if (orderCount === 0) {
                    displayResult(resultP, '查询订单成功: 该用户没有订单 (0)', 'info');
                } else {
                    displayResult(resultP, `查询到订单: 共 <span class="result-text-highlight-3">${orderCount}</span> 条`, 'success');
                    if (detailDiv) {
                        let tableHtml = '<table class="result-table"><thead><tr><th>状态</th><th>车次ID</th><th>出发站</th><th>出发时间</th><th>到达站</th><th>到达时间</th><th>价格</th><th>数量</th></tr></thead><tbody>';
                        for (let i = 1; i < orderLines.length; i++) {
                            const line = orderLines[i];
                            // 匹配模式，捕获状态和剩余部分
                            const match = line.match(/^\[(success|pending|refunded)\]\s(.+)$/);
                            if (match && match.length === 3) {
                                const status = match[1];
                                const restOfLine = match[2];
                                // 再解析剩余部分
                                const parts = restOfLine.split(' ');
                                const trainId = parts[0];
                                const fromStation = parts[1];
                                const leavingTime = `${parts[2]} ${parts[3]}`;
                                const toStation = parts[5];
                                const arrivingTime = `${parts[6]} ${parts[7]}`;
                                const price = parts[8];
                                const num = parts[9];

                                let statusColor = '';
                                if (status === 'success') statusColor = '#98c379'; // Green
                                else if (status === 'pending') statusColor = '#e5c07b'; // Yellow/Orange
                                else if (status === 'refunded') statusColor = '#e06c75'; // Red

                                tableHtml += `
                                    <tr>
                                        <td><span style="color:${statusColor}; font-weight: bold;">${status}</span></td>
                                        <td><span class="result-text-highlight-1">${trainId}</span></td>
                                        <td><span class="result-text-highlight-5">${fromStation}</span></td>
                                        <td><span class="result-text-highlight-2">${leavingTime}</span></td>
                                        <td><span class="result-text-highlight-5">${toStation}</span></td>
                                        <td><span class="result-text-highlight-2">${arrivingTime}</span></td>
                                        <td><span class="result-text-highlight-3">${price}</span></td>
                                        <td><span class="result-text-highlight-3">${num}</span></td>
                                    </tr>
                                `;
                            } else {
                                console.warn("Could not parse order line:", line);
                                tableHtml += `<tr><td colspan="8">解析错误: ${line}</td></tr>`;
                            }
                        }
                        tableHtml += '</tbody></table>';
                        detailDiv.innerHTML = tableHtml;
                    }
                }
                break;
            default:
                // 对于未特定处理的命令，只显示原始响应
                displayResult(resultP, `Server response: ${response}`, 'info');
                break;
        }
    }
}