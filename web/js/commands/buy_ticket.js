// web/js/commands/buy_ticket.js
import { setupCommandForm } from './_base_command.js'; // 修改导入路径

export function setupBuyTicketForm() {
    const paramFilter = (key, value, commandName) => {
        if (key === 'q' && commandName === 'buy_ticket') {
            return true;
        }
        return value.trim() !== '';
    };
    setupCommandForm('buy_ticket', paramFilter);
}