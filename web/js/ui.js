// web/js/ui.js
export function displayResult(resultPElement, message, type = 'info') {
    resultPElement.innerHTML = message; // 使用 innerHTML 允许传入高亮span
    resultPElement.classList.remove('success', 'failure', 'info', 'error');
    resultPElement.classList.add(type);
}

export function clearResultArea(moduleId) {
    const module = document.getElementById(moduleId);
    if (!module) return;

    const resultP = module.querySelector('.result-area p');
    const detailDiv = module.querySelector('.result-area .result-details, .result-area div[id$="Details"]'); // 查找复杂结果的显示区域

    // 清空详细结果区域
    if (detailDiv) detailDiv.innerHTML = '';

    if (resultP) {
        resultP.textContent = '';
        resultP.classList.remove('success', 'failure', 'info', 'error');
    }
}

// 模块切换和前端路由逻辑
export function showModule(moduleId, pushState = true) {
    // 隐藏所有模块
    document.querySelectorAll('.command-module').forEach(module => {
        module.classList.add('hidden');
        module.classList.remove('active');
    });
    // 显示目标模块
    const targetModule = document.getElementById(moduleId);
    if (targetModule) {
        targetModule.classList.remove('hidden');
        targetModule.classList.add('active');
        clearResultArea(moduleId); // 每次切换模块时清除结果区域
    }
    // 更新侧边栏选中状态
    document.querySelectorAll('.sidebar ul li a').forEach(link => link.classList.remove('active'));
    const activeLink = document.querySelector(`.sidebar ul li a[data-command-target="${moduleId}"]`);
    if (activeLink) {
        activeLink.classList.add('active');
    }


    // 更新 URL (前端路由)
    if (pushState) {
        const path = (moduleId === 'home' ? '/' : `/${moduleId}`);
        window.history.pushState({ module_id: moduleId }, '', path); // 使用小写下划线
    }
}

export function setupSidebarNavigation() {
    // 侧边栏点击事件
    document.querySelectorAll('.sidebar ul li a').forEach(link => {
        link.addEventListener('click', (e) => {
            e.preventDefault();
            const targetId = link.dataset.commandTarget;
            showModule(targetId, true); // 触发 pushState
        });
    });

    // 监听浏览器前进/后退事件
    window.addEventListener('popstate', (event) => {
        const state = event.state;
        if (state && state.module_id) { // 使用小写下划线
            showModule(state.module_id, false); // 不再 pushState，避免循环
        } else {
            // 如果 state 为空，通常是直接访问根URL或第一次加载
            showModule('home', false);
        }
    });

    // 首次加载时根据URL显示模块
    const initialPath = window.location.pathname.substring(1); // 移除开头的 '/'
    const initialModuleId = initialPath === '' ? 'home' : initialPath;
    // 检查这个 initialModuleId 是否对应一个实际的模块
    if (document.getElementById(initialModuleId)) {
        showModule(initialModuleId, false); // 首次加载不 pushState
    } else {
        showModule('home', false); // 如果URL无效，则回到首页
    }
}