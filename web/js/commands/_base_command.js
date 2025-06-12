// web/js/commands/_base_command.js
import { sendCommand } from '../websocket.js';
import { getNextTimestamp } from '../utils/timestamp.js';
import { displayResult } from '../ui.js';

/**
 * 设置通用命令表单的提交处理
 * @param {string} moduleId - 对应的HTML模块ID (例如 'add_user')
 * @param {Function} [paramFilter] - 可选的回调函数，用于过滤表单参数
 */
export function setupCommandForm(moduleId, paramFilter = null) {
    const form = document.querySelector(`#${moduleId} .command-form`);
    const resultP = document.getElementById(`${moduleId}Result`);

    if (!form || !resultP) {
        console.error(`Error: Form or result element not found for module ${moduleId}`);
        return;
    }

    form.addEventListener('submit', async (e) => {
        e.preventDefault();

        const formData = new FormData(form);
        const commandName = form.dataset.commandName; // "add_user"
        let commandParts = [];

        commandParts.push(`[${getNextTimestamp()}]`);
        commandParts.push(commandName);

        for (const [key, value] of formData.entries()) {
            const trimmedValue = value.trim();
            // 应用过滤函数，如果存在
            if (paramFilter) {
                if (paramFilter(key, trimmedValue, commandName)) {
                    commandParts.push(`-${key}`);
                    commandParts.push(trimmedValue);
                }
            } else {
                // 默认过滤空值
                if (trimmedValue !== '') {
                    commandParts.push(`-${key}`);
                    commandParts.push(trimmedValue);
                }
            }
        }

        const fullCommand = commandParts.join(' ');
        console.log("Sending command:", fullCommand);

        if (sendCommand(fullCommand, commandName)) {
            displayResult(resultP, `You sent: ${fullCommand}`, 'info');
        } else {
            displayResult(resultP, 'Error: WebSocket not connected. Command not sent.', 'error');
        }
    });
}