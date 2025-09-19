/**
 * ShortcutKeyViewer Browser Application
 * Multi-device shortcut key viewer with JSON configuration
 */

class ShortcutKeyViewer {
    constructor() {
        this.shortcuts = [];
        this.filteredShortcuts = [];
        this.currentOS = this.detectOS();
        
        this.init();
    }
    
    /**
     * Initialize the application
     */
    init() {
        this.setupEventListeners();
        this.setDefaultOSFilter();
        this.showLoading('設定ファイルを読み込んでください');
    }
    
    /**
     * Detect the current operating system
     */
    detectOS() {
        const userAgent = navigator.userAgent.toLowerCase();
        
        if (userAgent.includes('mac')) {
            return 'mac';
        } else if (userAgent.includes('win')) {
            return 'windows';
        } else if (userAgent.includes('linux')) {
            return 'linux';
        } else if (userAgent.includes('iphone') || userAgent.includes('ipad')) {
            return 'ios';
        } else if (userAgent.includes('android')) {
            return 'android';
        }
        
        return 'windows'; // Default fallback
    }
    
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
        const osFilter = document.getElementById('os-filter');
        osFilter.addEventListener('change', () => this.applyFilters());
        
        const programFilter = document.getElementById('program-filter');
        programFilter.addEventListener('change', () => this.applyFilters());
        
        const searchInput = document.getElementById('search-input');
        searchInput.addEventListener('input', () => this.applyFilters());
        
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
    
    /**
     * Set default OS filter to current OS
     */
    setDefaultOSFilter() {
        const osFilter = document.getElementById('os-filter');
        if (osFilter.querySelector(`option[value="${this.currentOS}"]`)) {
            osFilter.value = this.currentOS;
        }
    }
    
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
            
            // Validate and sort configuration
            this.shortcuts = config
                .filter(program => this.validateProgram(program))
                .sort((a, b) => (a.order || 999) - (b.order || 999));
            
            // Sort groups within each program
            this.shortcuts.forEach(program => {
                if (program.groups) {
                    program.groups.sort((a, b) => (a.order || 999) - (b.order || 999));
                }
            });
            
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
                if (!shortcut.action || !shortcut.keys || !shortcut.os) {
                    console.warn('無効なショートカット設定をスキップ:', shortcut);
                    return false;
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
        const programFilter = document.getElementById('program-filter');
        
        // Clear existing options except "all"
        while (programFilter.children.length > 1) {
            programFilter.removeChild(programFilter.lastChild);
        }
        
        // Add program options
        this.shortcuts.forEach(program => {
            const option = document.createElement('option');
            option.value = program.program;
            option.textContent = program.program;
            programFilter.appendChild(option);
        });
    }
    
    /**
     * Apply current filters
     */
    applyFilters() {
        const osFilter = document.getElementById('os-filter').value;
        const programFilter = document.getElementById('program-filter').value;
        const searchTerm = document.getElementById('search-input').value.toLowerCase();
        
        this.filteredShortcuts = this.shortcuts.filter(program => {
            // Program filter
            if (programFilter !== 'all' && program.program !== programFilter) {
                return false;
            }
            
            // Create filtered groups for this program
            const filteredGroups = program.groups.map(group => {
                const filteredShortcuts = group.shortcuts.filter(shortcut => {
                    // OS filter
                    if (osFilter !== 'all' && !shortcut.os.includes(osFilter)) {
                        return false;
                    }
                    
                    // Search filter
                    if (searchTerm) {
                        const searchableText = [
                            shortcut.action,
                            shortcut.description,
                            shortcut.keys.join(' ')
                        ].join(' ').toLowerCase();
                        
                        if (!searchableText.includes(searchTerm)) {
                            return false;
                        }
                    }
                    
                    return true;
                });
                
                return filteredShortcuts.length > 0 ? {
                    ...group,
                    shortcuts: filteredShortcuts
                } : null;
            }).filter(group => group !== null);
            
            return filteredGroups.length > 0 ? {
                ...program,
                groups: filteredGroups
            } : null;
        }).filter(program => program !== null);
        
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
     * Parse escape sequences in key strings
     * @@ -> @
     * @, -> ,
     */
    parseKeyEscapes(key) {
        return key.replace(/@@/g, '@').replace(/@,/g, ',');
    }
    
    /**
     * Check if a key is a special key (starts with @)
     */
    isSpecialKey(key) {
        return key.startsWith('@') && !key.startsWith('@@') && !key.startsWith('@,');
    }
    
    /**
     * Render a single key with proper styling
     */
    renderKey(key) {
        const parsedKey = this.parseKeyEscapes(key);
        const isSpecial = this.isSpecialKey(key);
        const keyClass = isSpecial ? 'key special-key' : 'key';
        const displayKey = isSpecial ? parsedKey.substring(1) : parsedKey; // Remove @ prefix for display
        
        return `<span class="${keyClass}">${this.escapeHtml(displayKey)}</span>`;
    }
    
    /**
     * Parse and render key sequences with comma separators
     */
    renderKeySequences(keys) {
        // First, check if the entire keys array represents a comma-separated sequence
        // For backward compatibility, keys can be an array of strings where each string might contain commas
        const processedKeys = [];
        
        for (const keyItem of keys) {
            if (typeof keyItem === 'string' && keyItem.includes(',')) {
                // This is a comma-separated sequence string
                const steps = keyItem.split(',').map(step => step.trim());
                const renderedSteps = steps.map(step => {
                    // Each step might have multiple keys (with + separator)
                    if (step.includes('+')) {
                        const simultaneousKeys = step.split('+').map(k => k.trim());
                        return simultaneousKeys.map(key => this.renderKey(key)).join('<span class="key-separator">+</span>');
                    } else {
                        return this.renderKey(step);
                    }
                });
                processedKeys.push(renderedSteps.join('<span class="key-separator">→</span>'));
            } else {
                // Regular key - add to the current simultaneous combination
                processedKeys.push(this.renderKey(keyItem));
            }
        }
        
        // Join all processed keys - if there were no comma sequences, use + separator
        // If there were comma sequences mixed with regular keys, use "then" separator
        const hasSequences = keys.some(key => typeof key === 'string' && key.includes(','));
        if (hasSequences && processedKeys.length > 1) {
            return processedKeys.join('<span class="sequence-separator">then</span>');
        } else {
            return processedKeys.join('<span class="key-separator">+</span>');
        }
    }
    
    /**
     * Render a single shortcut
     */
    renderShortcut(shortcut) {
        const keys = this.renderKeySequences(shortcut.keys);
        
        const osTags = shortcut.os
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