# Задание 3
## Запуск проекта с GitHub
cd gazpromtask3
git clone https://github.com/BogdanPustovalov/gazpromhacktask3
### Запуск ipynb файлов
Для запуска ipynb файлов (они не влияют на работу, но если есть заинтересованность в первоначальном коде на Python, то можно запустить) потребуется прописать следующие команды:
python -m venv .venv
.venv/Scripts/activate
pip install -r requirements.txt
### Запуск c++ файлов
#### temp_cpp - обработка графиков температуры
Для запуска в консоли нужно прописать следующие команды:
cd temp_cpp
g++ -g main_temp.cpp temp_model2.cpp -o main_temp.exe
.\main_temp.exe
#### press_cpp - обработка графиков давления
Для запуска в консоли нужно прописать следующие команды:
cd press_cpp
g++ -g main_press.cpp press_model2.cpp -o main_press.exe
.\main_press.exe
#### pos_cpp - обработка графиков положения КПВ
Для запуска в консоли нужно прописать следующие команды:
cd pos_cpp
g++ -g main_position.cpp pros_model2.cpp -o main_position.exe
.\main_position.exe
## Основная идея проекта
С помощью языка C++, методов машинного обучения и математических методов создать систему выявления аномалий для датчиков на производстве