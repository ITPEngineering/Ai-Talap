[[ru]](./README-ru.md)
[[en]](./README.md)

# Управляющее ПО ПЛК семейства Ai-Talap

Здесь находится программное обеспечение для конфигурации и
административного управления программируемыми логическими
контроллерами (ПЛК) серии Ai-Talap. Серия модульных ПЛК Ai-Talap
производится фирмой [IT-P Engineering](https://itpengineering.com).

## Требования к платформе

Программное обеспечение разрабатывалось под [Debian
GNU/Linux](http://debian.org), конкретно Debian 11 (bullseye), и
протестировано также под Debian 12 (bookworm).  Его должно быть
несложно адаптировать для любой платформы, где работает
[ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html).

Чтобы попробовать программное обеспечение в работе, потребуется ПЛК
(или УСО) серии Ai-Talap; как минимум, один из базовых модулей
(__AT241BC__, __AT241CPU__ или __AT340CPU__), подключенный к одной
локальной сети с управляющим компьютером.

## Установка

1. Установите Git:

   ```
   sudo apt-get update
   sudo apt-get install git git-gui

   ```
2. Клонируйте репозиторий

   ```
   git clone https://github.com/ITPEngineering/Ai-Talap
   cd Ai-Talap
   git submodule update --init --recursive

   ```
3. Запустите скрипт для автоматической установки

   ```
   export PATH="$PATH:$(pwd)/Install Software"
   chmod +x "Install Software/first_install.sh"
   first_install.sh
   
   ```

