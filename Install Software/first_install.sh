###Начало кода установки

#Шаг первый. Установка библиотек компилятора

sudo apt-get update
sudo apt-get install -y libsocket6-perl libjson-perl libyaml-perl
sudo apt-get install -y libdata-hexdump-perl
sudo apt-get install -y build-essential autoconf
sudo apt update && sudo apt install -y python3 python3-pip

echo "Установка библиотек компилятора выполнена успешно!"

#Шаг второй. Установка средства для сборки транслятора matiec

sudo apt-get update
sudo apt-get install -y build-essential autoconf
make -C matiec

echo "Установка средства для сборки транслятора matiec выполнена успешно!"

#Шаг третий. Установка среды разработки ESP­IDF
#All original source code in this repository is Copyright (C) 2015-2023 Espressif Systems. This source code is licensed under the Apache License 2.0 as described in the file LICENSE.

echo "deb [trusted=yes] https://dl.espressif.com/dl/eim/apt/ stable main" | sudo tee /etc/apt/sources.list.d/espressif.list
sudo apt update
sudo apt install -y eim
sudo tee /etc/yum.repos.d/espressif-eim.repo << 'EOF'
[eim]
name=ESP-IDF Installation Manager
baseurl=https://dl.espressif.com/dl/eim/rpm/$basearch
enabled=1
gpgcheck=0
EOF
sudo dnf install eim

eim install

echo "Установка среды разработки ESP­IDF выполнена успешно!"

#Шаг четвертый. Установка команд как исполняемых

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
AI_TALAP_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

cd "$AI_TALAP_DIR"
find bin -type f -exec chmod +x {} \;

#Шаг пятый. Автоматизация сборки ПО

cat << EOF >> ~/.bashrc

# ==== Ai-Talap setup ====
cd "$AI_TALAP_DIR"
export PATH="\$PATH:$AI_TALAP_DIR/bin"

make -C matiec

cd ~/esp/esp-idf
. \$HOME/esp/esp-idf/export.sh
cd

cd "$AI_TALAP_DIR"
# ==== end Ai-Talap ====

EOF

echo "Автоматизация сборки ПО выполнена успешно!"

###Конец кода установки