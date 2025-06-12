// web/js/commands/clean.js
import { sendCommand } from '../websocket.js';
import { getNextTimestamp } from '../utils/timestamp.js';
import { displayResult } from '../ui.js';

export function setupCleanButton() {
    const button = document.getElementById('confirmCleanData');
    const resultP = document.getElementById('cleanResult');

    if (!button || !resultP) {
        console.error('Error: Clean button or result element not found.');
        return;
    }

    button.addEventListener('click', () => {
        if (confirm('您确定要清除所有数据吗？此操作不可逆！')) {
            const commandName = 'clean';
            const fullCommand = `[${getNextTimestamp()}] ${commandName}`;
            console.log("Sending command:", fullCommand);

            if (sendCommand(fullCommand, commandName)) {
                displayResult(resultP, 'Clean command sent. Clearing data...', 'info');
            } else {
                displayResult(resultP, 'Error: WebSocket not connected. Command not sent.', 'error');
            }
        }
    });
}