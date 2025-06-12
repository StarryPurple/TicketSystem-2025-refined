// web/js/app.js
import { connectWebSocket } from './websocket.js';
import { showModule, setupSidebarNavigation } from './ui.js';
import { handleBackendResponse } from './response_handler.js'; // 修改导入路径

// 导入所有命令模块的 setup 函数
import { setupAddUserForm } from './commands/add_user.js';
import { setupLoginForm } from './commands/login.js';
import { setupLogoutForm } from './commands/logout.js';
import { setupQueryProfileForm } from './commands/query_profile.js';
import { setupModifyProfileForm } from './commands/modify_profile.js';
import { setupAddTrainForm } from './commands/add_train.js';
import { setupDeleteTrainForm } from './commands/delete_train.js';
import { setupReleaseTrainForm } from './commands/release_train.js';
import { setupQueryTrainForm } from './commands/query_train.js';
import { setupQueryTicketForm } from './commands/query_ticket.js';
import { setupQueryTransferForm } from './commands/query_transfer.js';
import { setupBuyTicketForm } from './commands/buy_ticket.js';
import { setupQueryOrderForm } from './commands/query_order.js';
import { setupRefundTicketForm } from './commands/refund_ticket.js';
import { setupCleanButton } from './commands/clean.js';
import { setupExitBackendButton } from './commands/exit_backend.js';


document.addEventListener('DOMContentLoaded', () => {
    const globalStatusDiv = document.getElementById('globalStatus');

    // 初始化 WebSocket 连接，并传入响应处理函数
    connectWebSocket(globalStatusDiv, handleBackendResponse);

    // 设置侧边栏导航和前端路由
    setupSidebarNavigation();

    // 默认显示首页
    showModule('home', false); // 首次加载不 pushState

    // 启用/禁用表单和按钮 (根据WebSocket连接状态)
    document.addEventListener('websocket:connected', () => {
        document.querySelectorAll('form button[type="submit"], #confirmExitBackend, #confirmCleanData').forEach(btn => btn.disabled = false);
    });
    document.addEventListener('websocket:disconnected', () => {
        document.querySelectorAll('form button[type="submit"], #confirmExitBackend, #confirmCleanData').forEach(btn => btn.disabled = true);
    });

    // 初始化所有命令表单的事件监听
    setupAddUserForm();
    setupLoginForm();
    setupLogoutForm();
    setupQueryProfileForm();
    setupModifyProfileForm();
    setupAddTrainForm();
    setupDeleteTrainForm();
    setupReleaseTrainForm();
    setupQueryTrainForm();
    setupQueryTicketForm();
    setupQueryTransferForm();
    setupBuyTicketForm();
    setupQueryOrderForm();
    setupRefundTicketForm();

    // 初始化特殊按钮的事件监听
    setupCleanButton();
    setupExitBackendButton();
});