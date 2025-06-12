// web/js/commands/exit_backend.js
import { sendCommand } from '../websocket.js';
import { getNextTimestamp } from '../utils/timestamp.js';
import { displayResult } from '../ui.js';
import { setAutoReconnect } from '../websocket.js'; // 用于关闭自动重连，如果需要

export function setupExitBackendButton() {
    const button = document.getElementById('confirmExitBackend');
    const resultP = document.getElementById('exitBackendResult');

    if (!button || !resultP) {
        console.error('Error: Exit backend button or result element not found.');
        return;
    }

    button.addEventListener('click', () => {
        if (confirm('您确定要终止C++后端服务吗？这会断开当前连接并尝试重连。')) {
            const commandName = 'exit'; // 指令是 'exit'
            const fullCommand = `[${getNextTimestamp()}] ${commandName}`;
            console.log("Sending command:", fullCommand);

            // setAutoReconnect(false); // 如果需要，可以取消此行注释以在退出后不自动重连

            if (sendCommand(fullCommand, 'exit_backend')) { // 这里的 commandName 仍然是 'exit_backend' 用于UI匹配
                displayResult(resultP, 'Exit command sent. Backend is terminating...', 'info');
            } else {
                displayResult(resultP, 'Error: WebSocket not connected. Command not sent.', 'error');
            }
        }
    });
}