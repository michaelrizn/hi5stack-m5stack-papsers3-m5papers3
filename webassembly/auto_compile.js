class AutoCompiler {
    constructor() {
        this.isCompiling = false;
        this.compilationQueue = [];
    }

    async checkAndCompileGames() {
        if (this.isCompiling) {
            return;
        }

        this.isCompiling = true;
        this.showCompilationStatus('🔍 Проверка игр...');

        try {
            const sourceGames = await this.getSourceGames();
            const compiledGames = await this.getCompiledGames();
            
            console.log('Найденные исходники:', sourceGames);
            console.log('Скомпилированные игры:', compiledGames);
            
            const gamesToCompile = sourceGames.filter(game => !compiledGames.includes(game));
            
            if (gamesToCompile.length === 0) {
                this.showCompilationStatus(`✅ Все игры актуальны (${sourceGames.length} игр)`);
                return;
            }
            
            this.showCompilationStatus(`📦 Найдено ${gamesToCompile.length} игр для компиляции: ${gamesToCompile.join(', ')}`);
            
            let successCount = 0;
            for (const game of gamesToCompile) {
                const success = await this.compileGame(game);
                if (success) successCount++;
            }
            
            if (successCount === gamesToCompile.length) {
                this.showCompilationStatus(`🎉 Компиляция завершена успешно (${successCount}/${gamesToCompile.length})`);
            } else {
                this.showCompilationStatus(`⚠️ Компиляция завершена с ошибками (${successCount}/${gamesToCompile.length})`);
            }
        } catch (error) {
            console.error('Ошибка при автоматической компиляции:', error);
        } finally {
            this.isCompiling = false;
        }
    }

    async getSourceGames() {
        const games = ['test', 'sudoku', 'minesweeper'];
        const sourceGames = [];
        
        for (const game of games) {
            try {
                const response = await fetch(`http://localhost:8001/check-source?game=${game}`);
                if (response.ok) {
                    const data = await response.json();
                    if (data.exists) {
                        sourceGames.push(game);
                    }
                }
            } catch (error) {
                console.log(`Не удалось проверить исходники для ${game}:`, error.message);
                sourceGames.push(game);
            }
        }
        
        return sourceGames;
    }

    async getCompiledGames() {
        const games = [];
        const gameNames = ['test', 'sudoku', 'minesweeper'];
        
        for (const name of gameNames) {
            try {
                const response = await fetch(`games/${name}/game.wasm`, { method: 'HEAD' });
                if (response.ok) {
                    games.push(name);
                }
            } catch (error) {
                console.log(`Скомпилированная игра ${name} не найдена`);
            }
        }
        
        return games;
    }

    async needsCompilation(gameName, compiledGames) {
        if (!compiledGames.includes(gameName)) {
            console.log(`Игра ${gameName} не скомпилирована`);
            return true;
        }

        try {
            const sourceResponse = await fetch(`../src/games/${gameName}/game.cpp`, { method: 'HEAD' });
            const compiledResponse = await fetch(`games/${gameName}/game.wasm`, { method: 'HEAD' });

            if (!sourceResponse.ok || !compiledResponse.ok) {
                return true;
            }

            const sourceDate = new Date(sourceResponse.headers.get('last-modified'));
            const compiledDate = new Date(compiledResponse.headers.get('last-modified'));

            if (sourceDate > compiledDate) {
                console.log(`Игра ${gameName} требует перекомпиляции`);
                return true;
            }
        } catch (error) {
            console.log(`Не удалось проверить даты для ${gameName}, компилируем`);
            return true;
        }

        return false;
    }

    async compileGame(gameName) {
        try {
            this.showCompilationStatus(`🔨 Компиляция ${gameName}...`);
            
            const response = await fetch(`http://localhost:8001/compile-game`, {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                },
                body: JSON.stringify({ game: gameName })
            });
            
            if (response.ok) {
                const result = await response.json();
                if (result.success) {
                    this.showCompilationStatus(`✅ ${gameName} скомпилирована успешно`);
                    return true;
                } else {
                    this.showCompilationStatus(`❌ Ошибка компиляции ${gameName}: ${result.error || 'Неизвестная ошибка'}`);
                    return false;
                }
            } else {
                const error = await response.text();
                this.showCompilationStatus(`❌ Ошибка компиляции ${gameName}: ${error}`);
                return false;
            }
        } catch (error) {
            console.log(`Ошибка при компиляции ${gameName}:`, error);
            this.showCompilationStatus(`⚠️ Автокомпиляция недоступна. Используйте: make ${gameName}`);
            return false;
        }
    }

    async fallbackCompile(gameName) {
        console.log(`Попытка компиляции через make для ${gameName}...`);
        
        try {
            if (window.electronAPI) {
                await window.electronAPI.runCommand(`cd webassembly && make ${gameName}`);
                console.log(`Игра ${gameName} скомпилирована через make`);
            } else {
                console.log(`Для компиляции ${gameName} выполните: cd webassembly && make ${gameName}`);
            }
        } catch (error) {
            console.error(`Ошибка fallback компиляции для ${gameName}:`, error);
        }
    }

    showCompilationStatus(message) {
        const statusEl = document.getElementById('compilation-status');
        if (statusEl) {
            statusEl.textContent = message;
            statusEl.style.display = 'block';
            setTimeout(() => {
                statusEl.style.display = 'none';
            }, 3000);
        }
    }
}

window.AutoCompiler = AutoCompiler;