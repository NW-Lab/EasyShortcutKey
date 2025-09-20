/**
 * Easy Shortcut Key - Browser Application
 * Multi-device shortcut key viewer with JSON configuration
 */

class ShortcutKeyViewer {
    constructor() {
        this.shortcuts = [];
        this.filteredShortcuts = [];
        this.activeProgramIndex = 0;
        this.activeGroupIndexMap = {}; // key: program index -> active group index
        this.activeGroupExpandedMap = {}; // key: program index -> boolean (show all groups)
        this.init();
    }
    
    /**
     * Initialize the application
     */
    init() {
        this.setupEventListeners();
        this.showLoading('設定ファイルを読み込んでください');
        // Automatically load default configuration on startup so the page shows content immediately
        // This will try external ../config/shortcuts.json first and fall back to embedded default-config
        this.loadDefaultConfig();
    }
    
    // OS自動判定は廃止したため関連メソッドは削除
    
    /**
     * Set up event listeners
     */
    setupEventListeners() {
        // No manual file input or settings button — browser uses bundled shortcuts.json only
        
        // Filters
    // No filters or search in simplified UI
        
        // Drag and drop support
        document.addEventListener('dragover', (e) => {
            e.preventDefault();
            e.stopPropagation();
        });
        
        document.addEventListener('drop', (e) => {
            e.preventDefault();
            e.stopPropagation();
            const files = e.dataTransfer.files;
            if (files.length > 0 && files[0].type === 'application/json') {
                this.loadConfigFromFile(files[0]);
            }
        });
    }
    
    // OSフィルタはUIから削除したため、このメソッドは不要
    
    /**
     * Handle file input change
     */
    handleFileLoad(event) {
        const file = event.target.files[0];
        if (file) {
            this.loadConfigFromFile(file);
        }
    }
    
    /**
     * Load configuration from file
     */
    loadConfigFromFile(file) {
        this.showLoading('設定ファイルを読み込み中...');
        
        const reader = new FileReader();
        reader.onload = (e) => {
            try {
                const config = JSON.parse(e.target.result);
                this.loadConfig(config);
            } catch (error) {
                this.showError('設定ファイルの形式が正しくありません: ' + error.message);
            }
        };
        reader.onerror = () => {
            this.showError('ファイルの読み込みに失敗しました');
        };
        reader.readAsText(file);
    }
    
    /**
     * Load default configuration
     */
    async loadDefaultConfig() {
        this.showLoading('デフォルト設定を読み込み中...');
        // If the page is opened via file://, many browsers block fetch/XHR due to CORS.
        // In that case prefer the embedded default-config (if present) to guarantee the UI works
        // when the user double-clicks index.html.
        try {
            if (typeof location !== 'undefined' && location.protocol === 'file:') {
                this.showLoading('file:// で開かれているため埋め込み設定を優先して読み込みます...');
                const embedded = document.getElementById('default-config');
                if (embedded && embedded.textContent && embedded.textContent.trim()) {
                    try {
                        const parsed = JSON.parse(embedded.textContent);
                        try { embedded.parentNode && embedded.parentNode.removeChild(embedded); } catch (e) {}
                        if (Array.isArray(parsed)) this.loadConfig(parsed);
                        else this.loadConfig([parsed]);
                        return;
                    } catch (e) {
                        console.debug('embedded parse failed', e);
                        // fall through to network attempts
                    }
                }
            }
        } catch (e) {
            // ignore and continue
        }
        
        let got = false;
        try {
            // Prefer the local browser copy so the app works when opened via file://
            // Try a few candidate URLs to account for different path resolution behaviors.
            const candidates = [
                'shortcuts.json',
                './shortcuts.json',
                `shortcuts.json?t=${Date.now()}`,
                `./shortcuts.json?t=${Date.now()}`
            ];
            console.debug('loadDefaultConfig: trying fetch candidates', candidates);
            for (const url of candidates) {
                try {
                    const response = await fetch(url);
                    if (!response.ok) continue;
                    // try to parse as JSON; if it fails, continue to next candidate
                    try {
                        const config = await response.json();
                        this.loadConfig(config);
                        got = true;
                        console.debug('loadDefaultConfig: loaded via fetch', url);
                        break;
                    } catch (e) {
                        continue;
                    }
                } catch (e) {
                    // network / file:// may throw; try next candidate
                    continue;
                }
            }
            if (got) return;
            // fallthrough to next fallback methods
        } catch (error) {
            console.debug('fetch-candidates threw', error);
        }

        // If we didn't successfully load via fetch candidates, run fallbacks (XHR / iframe / embedded)
        if (!got) {
            // Try synchronous XHR fallback for file:// environments where fetch may be blocked
            try {
                const xhr = new XMLHttpRequest();
                xhr.open('GET', 'shortcuts.json', false); // synchronous
                xhr.send(null);
                if (xhr.status === 200 || (xhr.status === 0 && xhr.responseText)) {
                    try {
                        const cfg = JSON.parse(xhr.responseText);
                        this.loadConfig(cfg);
                        return;
                    } catch (e) {
                        // parse error, continue to next fallback
                    }
                }
            } catch (xhrErr) {
                // ignore and fall back to next method
            }

            // iframe fallback
            try {
                const iframeResult = await new Promise((resolve) => {
                    let resolved = false;
                    try {
                        const iframe = document.createElement('iframe');
                        iframe.style.display = 'none';
                        iframe.src = 'shortcuts.json';
                        document.body.appendChild(iframe);

                        const cleanUp = () => {
                            try { iframe.parentNode && iframe.parentNode.removeChild(iframe); } catch (e) {}
                        };

                        const tryExtract = (doc) => {
                            try {
                                if (!doc) return null;
                                let txt = null;
                                try { txt = doc.body && doc.body.textContent; } catch (e) { txt = null; }
                                if (!txt) {
                                    try { txt = doc.documentElement && doc.documentElement.textContent; } catch (e) { txt = null; }
                                }
                                if (!txt) {
                                    try { txt = doc.documentElement && doc.documentElement.innerText; } catch (e) { txt = null; }
                                }
                                if (typeof txt === 'string') return txt;
                            } catch (e) {
                                return null;
                            }
                            return null;
                        };

                        const handleLoad = () => {
                            try {
                                const doc = iframe.contentDocument || (iframe.contentWindow && iframe.contentWindow.document);
                                const txt = tryExtract(doc);
                                if (txt && txt.trim()) {
                                    try {
                                        const parsed = JSON.parse(txt);
                                        resolved = true;
                                        cleanUp();
                                        resolve({ok: true, parsed});
                                        return;
                                    } catch (e) {
                                        console.debug('iframe JSON parse failed', e);
                                    }
                                }
                            } catch (e) {
                                console.debug('iframe access error', e);
                            }
                            resolved = true;
                            cleanUp();
                            resolve({ok: false});
                        };

                        const handleError = () => {
                            if (resolved) return;
                            resolved = true;
                            cleanUp();
                            resolve({ok: false});
                        };

                        iframe.addEventListener('load', handleLoad);
                        iframe.addEventListener('error', handleError);

                        setTimeout(() => {
                            if (resolved) return;
                            resolved = true;
                            cleanUp();
                            resolve({ok: false});
                        }, 2000);
                    } catch (e) {
                        if (!resolved) resolve({ok: false});
                    }
                });

                if (iframeResult && iframeResult.ok) {
                    const parsed = iframeResult.parsed;
                    console.debug('loadDefaultConfig: loaded via iframe fallback');
                    if (Array.isArray(parsed)) this.loadConfig(parsed);
                    else this.loadConfig([parsed]);
                    return;
                } else {
                    console.debug('loadDefaultConfig: iframe fallback failed');
                }
            } catch (iframeErr) {
                // ignore and continue to embedded fallback
            }

            // Embedded default JSON from the HTML page
            try {
                const embedded = document.getElementById('default-config');
                if (embedded && embedded.textContent && embedded.textContent.trim()) {
                    const parsed = JSON.parse(embedded.textContent);
                    try {
                        embedded.parentNode && embedded.parentNode.removeChild(embedded);
                    } catch (rmErr) {}
                    console.debug('loadDefaultConfig: loaded via embedded script');
                    if (Array.isArray(parsed)) {
                        this.loadConfig(parsed);
                    } else {
                        this.loadConfig([parsed]);
                    }
                    return;
                }

                // If the script tag doesn't exist (for example it's been commented out in the HTML),
                // attempt to extract the script content by scanning the page HTML.
                try {
                    const docHtml = document.documentElement && document.documentElement.innerHTML;
                    if (docHtml) {
                        const re = /(?:<!--\s*)?<script[^>]*id=["']default-config["'][^>]*type=["']application\/json["'][^>]*>([\s\S]*?)<\/script>(?:\s*-->)/i;
                        const m = docHtml.match(re);
                        if (m && m[1] && m[1].trim()) {
                            const parsed2 = JSON.parse(m[1]);
                            if (Array.isArray(parsed2)) {
                                this.loadConfig(parsed2);
                            } else {
                                this.loadConfig([parsed2]);
                            }
                            return;
                        }
                    }
                } catch (htmlErr) {
                    // ignore and fall through
                }
                // Electron-specific fallback: preload exposes readShortcutsJson
                try {
                    if (window && window.electronAPI && typeof window.electronAPI.readShortcutsJson === 'function') {
                        const txt = window.electronAPI.readShortcutsJson();
                        if (txt && txt.trim()) {
                            const parsed3 = JSON.parse(txt);
                            if (Array.isArray(parsed3)) {
                                this.loadConfig(parsed3);
                            } else {
                                this.loadConfig([parsed3]);
                            }
                            return;
                        }
                    }
                } catch (e) {
                    // ignore
                }
            } catch (err2) {
                // fallthrough to show error
            }

            this.showError('デフォルト設定の読み込みに失敗しました');
        }
    }
    
    /**
     * Load and validate configuration
     */
    loadConfig(config) {
        try {
            if (!Array.isArray(config)) {
                throw new Error('設定は配列である必要があります');
            }
            
            // Validate configuration and preserve original array order when `order` is missing.
            // Also honor `disEnable` at program level: skip programs where disEnable === true
            const validated = config
                .filter(program => !(program && program.disEnable === true))
                .filter(program => this.validateProgram(program));

            // Helper comparator that respects numeric `order` when present,
            // otherwise falls back to the original index captured on each item.
            const makeComparator = (indexKey) => (a, b) => {
                const aHas = typeof a.order === 'number' && isFinite(a.order);
                const bHas = typeof b.order === 'number' && isFinite(b.order);
                if (aHas && bHas) return a.order - b.order;
                if (aHas && !bHas) return -1;
                if (!aHas && bHas) return 1;
                // neither has order -> preserve original array order
                return (a[indexKey] || 0) - (b[indexKey] || 0);
            };

            // Tag original indices for top-level programs
            validated.forEach((p, idx) => { p.__origIndex = idx; });
            validated.sort(makeComparator('__origIndex'));

            // Now sort groups and shortcuts within each program while preserving original order if `order` is missing
            validated.forEach(program => {
                if (program.groups && Array.isArray(program.groups)) {
                    program.groups.forEach((g, gi) => { g.__origIndex = gi; });
                    program.groups.sort(makeComparator('__origIndex'));

                    program.groups.forEach(group => {
                        if (group.shortcuts && Array.isArray(group.shortcuts)) {
                            group.shortcuts.forEach((s, si) => { s.__origIndex = si; });
                            group.shortcuts.sort(makeComparator('__origIndex'));
                            // clean up temporary shortcut indices
                            group.shortcuts.forEach(s => { delete s.__origIndex; });
                        }
                        // clean up temporary group index
                        delete group.__origIndex;
                    });
                }
                // clean up temporary program index
                delete program.__origIndex;
            });

            this.shortcuts = validated;
            
            this.updateProgramFilter();
            this.applyFilters();
            // If a notable shortcut exists (like Open Chat), auto-open its program/group so it's visible
            try {
                this.openFirstMatchingShortcut && this.openFirstMatchingShortcut('Open Chat');
            } catch (e) {
                // ignore
            }
            this.hideLoading();
            
        } catch (error) {
            this.showError('設定の検証に失敗しました: ' + error.message);
        }
    }
    
    /**
     * Validate program configuration
     */
    validateProgram(program) {
            if (!program.appName || !program.groups) {
                console.warn('無効なプログラム設定をスキップ:', program);
                return false;
            }
        
        // Validate groups
        program.groups = program.groups.filter(group => {
            // honor group-level disEnable flag: skip if true
            if (group && group.disEnable === true) return false;

            if (!group.groupName || !group.shortcuts) {
                console.warn('無効なグループ設定をスキップ:', group);
                return false;
            }

            // Validate shortcuts
            // honor shortcut-level disEnable flag: skip if true
            group.shortcuts = group.shortcuts.filter(shortcut => {
                if (shortcut && shortcut.disEnable === true) return false;
                // Allow shortcuts that have either keys (legacy) or structured steps
                if (!shortcut.action || (!shortcut.keys && !Array.isArray(shortcut.steps))) {
                    console.warn('無効なショートカット設定をスキップ:', shortcut);
                    return false;
                }

                // Make os optional; ensure it's an array for later rendering
                if (!Array.isArray(shortcut.os)) {
                    shortcut.os = [];
                }

                return true;
            });

            return group.shortcuts.length > 0;
        });
        
        return program.groups.length > 0;
    }
    
    /**
     * Update program filter options
     */
    updateProgramFilter() {
        // Program filter removed; nothing to update in the UI
    }
    
    /**
     * Apply current filters
     */
    applyFilters() {
        // Simplified: no filters/search. Just render all programs as already sorted by order.
        this.filteredShortcuts = this.shortcuts;
        this.render();
    }
    
    /**
     * Render the application
     */
    render() {
        const container = document.getElementById('programs-container');
        const tabsContainer = document.getElementById('tabs-container');

        if (this.filteredShortcuts.length === 0) {
            tabsContainer.innerHTML = '';
            container.innerHTML = '<div class="loading">該当するショートカットが見つかりませんでした</div>';
            return;
        }

            tabsContainer.innerHTML = this.filteredShortcuts
                .map((program, idx) => `\n                <button class="tab ${idx === this.activeProgramIndex ? 'active' : ''}" data-idx="${idx}">${this.escapeHtml(program.appName)}</button>\n            `)
            .join('');

        // Attach click handlers for tabs
        Array.from(tabsContainer.querySelectorAll('.tab')).forEach(btn => {
            btn.addEventListener('click', (e) => {
                const idx = parseInt(e.currentTarget.getAttribute('data-idx'), 10);
                if (!isNaN(idx)) {
                    this.activeProgramIndex = idx;
                    this.render();
                }
            });
        });

        // Render only the active program
        const active = this.filteredShortcuts[this.activeProgramIndex] || this.filteredShortcuts[0];
        container.innerHTML = this.renderProgram(active);

        // Attach group tab and expand handlers (per-program)
        const programIndex = this.activeProgramIndex;
    // include expand control as part of the tab-like buttons so it receives the same handling
    const groupTabButtons = container.querySelectorAll('.group-tab, .group-expand');
        Array.from(groupTabButtons).forEach(btn => {
            btn.addEventListener('click', (e) => {
                const isExpand = btn.classList.contains('group-expand');
                if (isExpand) {
                    // Toggle expanded state
                    const cur = !!this.activeGroupExpandedMap[programIndex];
                    this.activeGroupExpandedMap[programIndex] = !cur;
                    this.render();
                    return;
                }

                const gidxAttr = btn.getAttribute('data-gidx');
                if (gidxAttr != null) {
                    const gidx = parseInt(gidxAttr, 10);
                    if (!isNaN(gidx)) {
                        // Clicking a group tab should cancel expanded mode and select that group
                        this.activeGroupExpandedMap[programIndex] = false;
                        this.activeGroupIndexMap[programIndex] = gidx;
                        this.render();
                    }
                }
            });
        });
    }
    
    /**
     * Render a single program
     */
    renderProgram(program) {
        // Tabs already show the program title; render group tabs and only the active group's content.
        const progIdx = this.activeProgramIndex;
        const groups = Array.isArray(program.groups) ? program.groups : [];
        const activeGroupIndex = (this.activeGroupIndexMap[progIdx] != null) ? this.activeGroupIndexMap[progIdx] : 0;
    const expanded = !!this.activeGroupExpandedMap[progIdx];
    // when expanded, no individual group-tab should be shown as active
    const groupTabsHtml = groups.map((g, gi) => `\n                <button class="group-tab ${(!expanded && gi === activeGroupIndex) ? 'active' : ''}" data-gidx="${gi}">${this.escapeHtml(g.groupName)}</button>\n            `).join('');

        // When expanded, render all groups' contents; otherwise render only active group
        const contentHtml = expanded
            ? groups.map(g => this.renderGroup(g)).join('')
            : (groups[activeGroupIndex] ? this.renderGroup(groups[activeGroupIndex]) : '');

        // Expand button placed to the left of group tabs
    const expandBtnHtml = `<button class="group-expand ${expanded ? 'active' : ''}" title="展開/折畳">${expanded ? '▾' : '▸'}</button>`;

        return `
            <div class="program compact ${expanded ? 'expanded' : ''}">
                <div class="group-tabs-wrap">${expandBtnHtml}<div class="group-tabs">${groupTabsHtml}</div></div>
                <div class="program-content">
                    ${contentHtml}
                </div>
            </div>
        `;
    }
    
    /**
     * Render a single group
     */
    renderGroup(group) {
        // Render group header and its shortcuts. When used in expanded view, the header
        // helps identify which group the shortcuts belong to.
        return `
            <div class="group">
                <div class="group-header">${this.escapeHtml(group.groupName)}</div>
                <div class="shortcuts">
                    ${group.shortcuts.map(shortcut => this.renderShortcut(shortcut)).join('')}
                </div>
            </div>
        `;
    }
    
    /**
     * Render a single shortcut
     */
    renderShortcut(shortcut) {
        // Render keys if present (legacy/simple case)
        const keys = Array.isArray(shortcut.keys) ?
            shortcut.keys.map(key => `<span class="key">${this.escapeHtml(key)}</span>`)
                .join('<span class="key-separator">+</span>') : '';

        const osTags = (Array.isArray(shortcut.os) ? shortcut.os : [])
            .map(os => `<span class="os-tag ${os}">${os.toUpperCase()}</span>`)
            .join('');

        // Render structured steps if present
        let stepsHtml = '';
        if (Array.isArray(shortcut.steps) && shortcut.steps.length > 0) {
            stepsHtml = '<ol class="shortcut-steps">' +
                shortcut.steps.map((s, i) => {
                    const stepKeys = Array.isArray(s.keys) ? s.keys.map(k => `<span class="key">${this.escapeHtml(k)}</span>`).join('<span class="key-separator">+</span>') : '';
                    // Prefer explicit description; if missing, use action only when it's not a generic 'Seq' label
                    let stepDesc = '';
                    if (s.description) stepDesc = this.escapeHtml(s.description);
                    else if (s.action && typeof s.action === 'string') {
                        const a = s.action.trim();
                        if (a.toLowerCase() !== 'seq') stepDesc = this.escapeHtml(a);
                    }
                    // For browser UI we only show the step keys to keep the UI compact; descriptions are kept in JSON
                    return `<li class="shortcut-step"><div class="step-keys">${stepKeys}</div></li>`;
                }).join('') + '</ol>';
        }

        return `
            <div class="shortcut">
                <div class="shortcut-info">
                    <div class="shortcut-action">${this.escapeHtml(shortcut.action)}</div>
                    <div class="shortcut-description">${this.escapeHtml(shortcut.description)}</div>
                    <div class="shortcut-os">${osTags}</div>
                </div>
                <div class="shortcut-keys">${keys}</div>
                ${stepsHtml}
            </div>
        `;
    }
    
    /**
     * Show loading message
     */
    showLoading(message) {
        const loading = document.getElementById('loading');
        const error = document.getElementById('error');
        const container = document.getElementById('programs-container');
        
        loading.textContent = message;
        loading.style.display = 'block';
        error.style.display = 'none';
        container.innerHTML = '';
    }
    
    /**
     * Hide loading message
     */
    hideLoading() {
        const loading = document.getElementById('loading');
        loading.style.display = 'none';
    }
    
    /**
     * Show error message
     */
    showError(message) {
        const loading = document.getElementById('loading');
        const error = document.getElementById('error');
        const container = document.getElementById('programs-container');
        
        loading.style.display = 'none';
        error.textContent = message;
        error.style.display = 'block';
        container.innerHTML = '';
    }
    
    /**
     * Escape HTML to prevent XSS
     */
    escapeHtml(text) {
        const div = document.createElement('div');
        div.textContent = text;
        return div.innerHTML;
    }
    /**
     * Find the first shortcut whose action contains the provided substring and make it visible
     */
    openFirstMatchingShortcut(substring) {
        if (!substring || !Array.isArray(this.shortcuts)) return false;
        for (let pi = 0; pi < this.shortcuts.length; pi++) {
            const prog = this.shortcuts[pi];
            if (!prog || !Array.isArray(prog.groups)) continue;
            for (let gi = 0; gi < prog.groups.length; gi++) {
                const grp = prog.groups[gi];
                if (!grp || !Array.isArray(grp.shortcuts)) continue;
                for (let si = 0; si < grp.shortcuts.length; si++) {
                    const s = grp.shortcuts[si];
                    if (s && s.action && typeof s.action === 'string' && s.action.indexOf(substring) !== -1) {
                        this.activeProgramIndex = pi;
                        this.activeGroupIndexMap[pi] = gi;
                        this.activeGroupExpandedMap[pi] = false;
                        // re-render to reflect selection
                        try { this.render(); } catch (e) {}
                        return true;
                    }
                }
            }
        }
        return false;
    }
}

// Initialize the application when DOM is loaded
document.addEventListener('DOMContentLoaded', () => {
    new ShortcutKeyViewer();
});