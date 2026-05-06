class M5DisplayEmulator {
    constructor(canvasId) {
        this.canvas = document.getElementById(canvasId);
        this.ctx = this.canvas.getContext('2d');
        this.width = 540;
        this.height = 960;
        this.canvas.width = this.width;
        this.canvas.height = this.height;
        
        this.currentFont = 'Arial';
        this.currentFontSize = 12;
        this.currentTextColor = '#000000';
        this.currentBgColor = '#FFFFFF';
        this.cursorX = 0;
        this.cursorY = 0;
        
        this.setupCanvas();
    }
    
    setupCanvas() {
        this.ctx.fillStyle = this.currentBgColor;
        this.ctx.fillRect(0, 0, this.width, this.height);
        this.ctx.textBaseline = 'top';
    }
    
    fillRect(x, y, width, height, color) {
        this.ctx.fillStyle = this.colorToHex(color);
        this.ctx.fillRect(x, y, width, height);
    }
    
    setCursor(x, y) {
        this.cursorX = x;
        this.cursorY = y;
    }
    
    setTextColor(textColor, bgColor = null) {
        this.currentTextColor = this.colorToHex(textColor);
        if (bgColor !== null) {
            this.currentBgColor = this.colorToHex(bgColor);
        }
    }
    
    setTextSize(size) {
        this.currentFontSize = size * 8;
        this.ctx.font = `${this.currentFontSize}px ${this.currentFont}`;
    }
    
    setFont(font) {
        this.currentFont = 'Arial';
        this.ctx.font = `${this.currentFontSize}px ${this.currentFont}`;
    }
    
    print(text) {
        if (this.currentBgColor) {
            const metrics = this.ctx.measureText(text);
            this.ctx.fillStyle = this.currentBgColor;
            this.ctx.fillRect(this.cursorX, this.cursorY, metrics.width, this.currentFontSize);
        }
        
        this.ctx.fillStyle = this.currentTextColor;
        this.ctx.fillText(text, this.cursorX, this.cursorY);
        
        const textWidth = this.ctx.measureText(text).width;
        this.cursorX += textWidth;
    }
    
    drawLine(x1, y1, x2, y2, color) {
        this.ctx.strokeStyle = this.colorToHex(color);
        this.ctx.lineWidth = 1;
        this.ctx.beginPath();
        this.ctx.moveTo(x1, y1);
        this.ctx.lineTo(x2, y2);
        this.ctx.stroke();
    }
    
    drawRect(x, y, width, height, color) {
        this.ctx.strokeStyle = this.colorToHex(color);
        this.ctx.lineWidth = 1;
        this.ctx.strokeRect(x, y, width, height);
    }
    
    drawCircle(x, y, radius, color) {
        this.ctx.strokeStyle = this.colorToHex(color);
        this.ctx.lineWidth = 1;
        this.ctx.beginPath();
        this.ctx.arc(x, y, radius, 0, 2 * Math.PI);
        this.ctx.stroke();
    }
    
    fillCircle(x, y, radius, color) {
        this.ctx.fillStyle = this.colorToHex(color);
        this.ctx.beginPath();
        this.ctx.arc(x, y, radius, 0, 2 * Math.PI);
        this.ctx.fill();
    }
    
    textWidth(text) {
        return this.ctx.measureText(text).width;
    }

    fontHeight() {
        return this.currentFontSize;
    }

    drawString(text, x, y) {
        this.ctx.fillStyle = this.currentTextColor;
        this.ctx.font = `${this.currentFontSize}px ${this.currentFont}`;
        this.ctx.fillText(text, x, y);
    }

    setColor(color) {
        this.ctx.strokeStyle = this.colorToHex(color);
        this.ctx.fillStyle = this.colorToHex(color);
    }

    clear(color = '#FFFFFF') {
        this.fillRect(0, 0, this.width, this.height, color);
    }
    
    colorToHex(color) {
        if (typeof color === 'string') {
            return color;
        }
        
        const colorMap = {
            0x0000: '#000000',
            0xFFFF: '#FFFFFF', 
            0xF800: '#FF0000',
            0x07E0: '#00FF00',
            0x001F: '#0000FF',
            0xFFE0: '#FFFF00',
            0xF81F: '#FF00FF',
            0x07FF: '#00FFFF',
            0x8410: '#808080',
            0xC618: '#C0C0C0'
        };
        
        if (colorMap[color]) {
            return colorMap[color];
        }
        
        const r = ((color >> 11) & 0x1F) * 8;
        const g = ((color >> 5) & 0x3F) * 4;
        const b = (color & 0x1F) * 8;
        
        return `rgb(${r}, ${g}, ${b})`;
    }
}

class M5TouchEmulator {
    constructor(canvas, touchCallback) {
        this.canvas = canvas;
        this.touchCallback = touchCallback;
        this.setupEventListeners();
    }
    
    setCallback(callback) {
        this.touchCallback = callback;
    }
    
    setupEventListeners() {
        this.canvas.addEventListener('click', (e) => {
            const rect = this.canvas.getBoundingClientRect();
            const scaleX = this.canvas.width / rect.width;
            const scaleY = this.canvas.height / rect.height;
            
            const x = Math.floor((e.clientX - rect.left) * scaleX);
            const y = Math.floor((e.clientY - rect.top) * scaleY);
            
            if (this.touchCallback) {
                this.touchCallback(1, x, y);
            }
        });
        
        this.canvas.addEventListener('mousedown', (e) => {
            const rect = this.canvas.getBoundingClientRect();
            const scaleX = this.canvas.width / rect.width;
            const scaleY = this.canvas.height / rect.height;
            
            const x = Math.floor((e.clientX - rect.left) * scaleX);
            const y = Math.floor((e.clientY - rect.top) * scaleY);
            
            if (this.touchCallback) {
                this.touchCallback(0, x, y);
            }
        });
    }
}

class M5Emulator {
    constructor(canvasId) {
        this.display = new M5DisplayEmulator(canvasId);
        this.touch = new M5TouchEmulator(this.display.canvas, null);
        this.gameModule = null;
        
        // Make emulator globally accessible for WASM
        window.emulator = this;
    }

    setTouchCallback(callback) {
        this.touch.setCallback(callback);
    }
    
    loadGame(wasmPath, gameNamespace) {
        return new Promise((resolve, reject) => {
            const script = document.createElement('script');
            script.src = wasmPath;
            script.onload = async () => {
                try {
                    if (typeof Module !== 'undefined') {
                        // Module is a factory function, call it to get the actual module
                        this.gameModule = await Module();
                        
                        // Wait for the module to be fully initialized
                        if (this.gameModule.ready) {
                            await this.gameModule.ready;
                        }
                        
                        this.setupGameBindings(gameNamespace);
                        resolve(this.gameModule);
                    } else {
                        reject(new Error('Module not found'));
                    }
                } catch (error) {
                    reject(error);
                }
            };
            script.onerror = reject;
            document.head.appendChild(script);
        });
    }
    
    setupGameBindings(gameNamespace) {
        if (this.gameModule && this.gameModule.cwrap) {
            const initGame = this.gameModule.cwrap(`${gameNamespace}_initGame`, null, []);
            const drawGameScreen = this.gameModule.cwrap(`${gameNamespace}_drawGameScreen`, null, []);
            const handleTouch = this.gameModule.cwrap(`${gameNamespace}_handleTouch`, null, ['number', 'number', 'number']);
            const resetGame = this.gameModule.cwrap(`${gameNamespace}_resetGame`, null, []);
            
            this.initGame = initGame;
            this.drawGameScreen = drawGameScreen;
            this.handleTouch = handleTouch;
            this.resetGame = resetGame;
            
            this.setTouchCallback((touchType, x, y) => {
                if (this.handleTouch) {
                    this.handleTouch(touchType, x, y);
                    if (this.drawGameScreen) {
                        this.drawGameScreen();
                    }
                }
            });
        } else {
            console.error('Game module not ready or cwrap not available');
        }
    }
}

const TFT_BLACK = 0x0000;
const TFT_WHITE = 0xFFFF;
const TFT_RED = 0xF800;
const TFT_GREEN = 0x07E0;
const TFT_BLUE = 0x001F;
const TFT_YELLOW = 0xFFE0;
const TFT_MAGENTA = 0xF81F;
const TFT_CYAN = 0x07FF;
const TFT_DARKGREY = 0x8410;
const TFT_LIGHTGREY = 0xC618;

window.M5Emulator = M5Emulator;
window.TFT_BLACK = TFT_BLACK;
window.TFT_WHITE = TFT_WHITE;
window.TFT_RED = TFT_RED;
window.TFT_GREEN = TFT_GREEN;
window.TFT_BLUE = TFT_BLUE;
window.TFT_YELLOW = TFT_YELLOW;
window.TFT_MAGENTA = TFT_MAGENTA;
window.TFT_CYAN = TFT_CYAN;
window.TFT_DARKGREY = TFT_DARKGREY;
window.TFT_LIGHTGREY = TFT_LIGHTGREY;