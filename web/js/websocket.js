// web/js/websocket.js
const wsUrl = 'ws://localhost:8765';
let wsInstance;
let autoReconnect = true;
let globalStatusDiv;
let lastCommandName = ''; // 跟踪最后发送的命令，用于响应处理

export function connectWebSocket(statusDiv, handlerCallback) {
    globalStatusDiv = statusDiv;
    // 当连接成功或断开时，我们发送自定义事件
    document.addEventListener('websocket:send_command', (event) => {
        lastCommandName = event.detail.command_name; // 记录发送的命令名 (使用小写下划线)
    });

    wsInstance = new WebSocket(wsUrl);

    wsInstance.onopen = () => {
        console.log('WebSocket connected');
        globalStatusDiv.textContent = 'Connected';
        globalStatusDiv.classList.remove('disconnected', 'error');
        globalStatusDiv.classList.add('connected');
        document.dispatchEvent(new CustomEvent('websocket:connected')); // 触发事件
    };

    wsInstance.onmessage = (event) => {
        console.log('Received from server:', event.data);
        // 使用 lastCommandName 来传递给响应处理函数
        handlerCallback(event.data, lastCommandName);
        lastCommandName = ''; // 响应处理完毕后清空
    };

    wsInstance.onclose = () => {
        console.log('WebSocket disconnected.');
        globalStatusDiv.textContent = 'Disconnected';
        globalStatusDiv.classList.remove('connected');
        globalStatusDiv.classList.add('disconnected');
        document.dispatchEvent(new CustomEvent('websocket:disconnected')); // 触发事件

        if (autoReconnect) {
            console.log('Attempting to reconnect in 5 seconds...');
            globalStatusDiv.textContent = 'Disconnected. Reconnecting...';
            setTimeout(() => connectWebSocket(statusDiv, handlerCallback), 5000); // 重新连接时传递回调
        } else {
            globalStatusDiv.textContent = 'Disconnected. Session ended.';
        }
    };

    wsInstance.onerror = (error) => {
        console.error('WebSocket error:', error);
        globalStatusDiv.textContent = 'Error. Check server console.';
        globalStatusDiv.classList.remove('connected', 'disconnected');
        globalStatusDiv.classList.add('error');
    };

    return wsInstance;
}

export function sendCommand(command, commandName) {
    if (wsInstance && wsInstance.readyState === WebSocket.OPEN) {
        document.dispatchEvent(new CustomEvent('websocket:send_command', { detail: { command_name: commandName } })); // 使用小写下划线
        wsInstance.send(command);
        return true;
    } else {
        console.warn('WebSocket not open. Command not sent.');
        return false;
    }
}

export function setAutoReconnect(value) {
    autoReconnect = value;
}