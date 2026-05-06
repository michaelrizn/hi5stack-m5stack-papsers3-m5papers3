#!/bin/bash

echo "🎮 Запуск M5Stack Games Emulator"
echo "================================"

cd "$(dirname "$0")"

echo "Проверка зависимостей..."
if ! command -v emcc &> /dev/null; then
    echo "❌ Emscripten не найден. Установите Emscripten SDK."
    exit 1
fi

if ! command -v python3 &> /dev/null; then
    echo "❌ Python3 не найден. Установите Python 3."
    exit 1
fi

echo "✅ Зависимости найдены"

echo "Создание директорий для игр..."
mkdir -p games/test games/sudoku games/minesweeper

echo "Запуск сервера компиляции на порту 8001..."
python3 compile_server.py 8001 &
COMPILE_PID=$!

echo "Ожидание запуска сервера компиляции..."
sleep 2

echo "Запуск веб-сервера на порту 8000..."
python3 -m http.server 8000 &
WEB_PID=$!

echo "Ожидание запуска веб-сервера..."
sleep 2

echo ""
echo "🚀 Эмулятор запущен!"
echo "📱 Откройте в браузере: http://localhost:8000"
echo "🔧 Сервер компиляции: http://localhost:8001"
echo ""
echo "Для остановки нажмите Ctrl+C"
echo ""

cleanup() {
    echo ""
    echo "🛑 Остановка серверов..."
    kill $COMPILE_PID 2>/dev/null
    kill $WEB_PID 2>/dev/null
    echo "✅ Серверы остановлены"
    exit 0
}

trap cleanup SIGINT SIGTERM

if command -v open &> /dev/null; then
    echo "🌐 Открытие браузера..."
    open http://localhost:8000
elif command -v xdg-open &> /dev/null; then
    echo "🌐 Открытие браузера..."
    xdg-open http://localhost:8000
fi

wait