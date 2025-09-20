/**
 * Easy Shortcut Key - Browser Application
 * Multi-device shortcut key viewer with JSON configuration
 */

class ShortcutKeyViewer {
    constructor() {
        this.shortcuts = [];
        this.filteredShortcuts = [];
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
        // File input
        const fileInput = document.getElementById('config-file');
        fileInput.addEventListener('change', (e) => this.handleFileLoad(e));
        
        // Load default button
        const loadDefaultBtn = document.getElementById('load-default');
        loadDefaultBtn.addEventListener('click', () => this.loadDefaultConfig());
        
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
        
        try {
            const response = await fetch('../config/shortcuts.json');
            if (!response.ok) {
                throw new Error(`HTTP ${response.status}: ${response.statusText}`);
            }
            const config = await response.json();
            this.loadConfig(config);
        } catch (error) {
            // If fetching the external config fails (e.g., file:// restrictions),
            // try to read embedded default JSON from the HTML page.
            try {
                const embedded = document.getElementById('default-config');
                if (embedded && embedded.textContent.trim()) {
                    const parsed = JSON.parse(embedded.textContent);
                    if (Array.isArray(parsed)) {
                        this.loadConfig(parsed);
                    } else {
                        this.loadConfig([parsed]);
                    }
                    return;
                }
            } catch (err2) {
                // fallthrough to show error
            }

            this.showError('デフォルト設定の読み込みに失敗しました: ' + error.message);
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
            const validated = config.filter(program => this.validateProgram(program));

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
            this.hideLoading();
            
        } catch (error) {
            this.showError('設定の検証に失敗しました: ' + error.message);
        }
    }
    
    /**
     * Validate program configuration
     */
    validateProgram(program) {
        if (!program.program || !program.groups) {
            console.warn('無効なプログラム設定をスキップ:', program);
            return false;
        }
        
        // Validate groups
        program.groups = program.groups.filter(group => {
            if (!group.groupName || !group.shortcuts) {
                console.warn('無効なグループ設定をスキップ:', group);
                return false;
            }
            
            // Validate shortcuts
            group.shortcuts = group.shortcuts.filter(shortcut => {
                if (!shortcut.action || !shortcut.keys) {
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
        
        if (this.filteredShortcuts.length === 0) {
            container.innerHTML = '<div class="loading">該当するショートカットが見つかりませんでした</div>';
            return;
        }
        
        container.innerHTML = this.filteredShortcuts
            .map(program => this.renderProgram(program))
            .join('');
    }
    
    /**
     * Render a single program
     */
    renderProgram(program) {
        return `
            <div class="program">
                <div class="program-header">
                    <h2>${this.escapeHtml(program.program)}</h2>
                    ${program.version ? `<div class="program-version">${this.escapeHtml(program.version)}</div>` : ''}
                </div>
                <div class="program-content">
                    ${program.groups.map(group => this.renderGroup(group)).join('')}
                </div>
            </div>
        `;
    }
    
    /**
     * Render a single group
     */
    renderGroup(group) {
        return `
            <div class="group">
                <div class="group-header">
                    <div class="group-name">${this.escapeHtml(group.groupName)}</div>
                    ${group.description ? `<div class="group-description">${this.escapeHtml(group.description)}</div>` : ''}
                </div>
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
        const keys = shortcut.keys
            .map(key => `<span class="key">${this.escapeHtml(key)}</span>`)
            .join('<span class="key-separator">+</span>');
        
        const osTags = (Array.isArray(shortcut.os) ? shortcut.os : [])
            .map(os => `<span class="os-tag ${os}">${os.toUpperCase()}</span>`)
            .join('');
        
        return `
            <div class="shortcut">
                <div class="shortcut-info">
                    <div class="shortcut-action">${this.escapeHtml(shortcut.action)}</div>
                    <div class="shortcut-description">${this.escapeHtml(shortcut.description)}</div>
                    <div class="shortcut-os">${osTags}</div>
                </div>
                <div class="shortcut-keys">${keys}</div>
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
}

// Initialize the application when DOM is loaded
document.addEventListener('DOMContentLoaded', () => {
    new ShortcutKeyViewer();
});