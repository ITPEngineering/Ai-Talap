###Начало кода установки

#Шаг первый. Установка библиотек компилятора

sudo apt-get update
sudo apt-get install libsocket6-perl libjson-perl libyaml-perl
sudo apt-get install libdata-hexdump-perl
sudo apt-get install build-essential autoconf
sudo apt update && sudo apt install -y python3 python3-pip

echo "Установка библиотек компилятора выполнена успешно!"

#Шаг второй. Установка средства для сборки транслятора matiec

sudo apt-get update
sudo apt-get install build-essential autoconf
make -C matiec

echo "Установка средства для сборки транслятора matiec выполнена успешно!"

#Шаг третий. Установка среды разработки ESP­IDF
#All original source code in this repository is Copyright (C) 2015-2023 Espressif Systems. This source code is licensed under the Apache License 2.0 as described in the file LICENSE.

echo "deb [trusted=yes] https://dl.espressif.com/dl/eim/apt/ stable main" | sudo tee /etc/apt/sources.list.d/espressif.list
sudo apt update
sudo apt install eim
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

#Шаг четвертый. Автоматизация сборки ПО

cat << 'EOF' >> ~/.bashrc

# ==== Ai-Talap setup ====
cd ~/Ai-Talap
export PATH="$PATH:$(pwd)/bin"

make -C matiec

cd ~/esp/esp-idf
. $HOME/esp/esp-idf/export.sh
cd

cd ~/Ai-Talap
# ==== end Ai-Talap ====

EOF

echo "Автоматизация сборки ПО выполнена успешно!"

###Конец кода установки