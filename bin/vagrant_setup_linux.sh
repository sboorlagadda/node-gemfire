#!/bin/bash

GEMFIRE_VERSION=9.5.1
NATIVE_CLIENT_VERSION=9.2.1
GEMFIRE_SERVER_FILENAME="pivotal-gemfire-${GEMFIRE_VERSION}.zip"
GEMFIRE_DIRECTORY="/opt/pivotal/gemfire/pivotal-gemfire-${GEMFIRE_VERSION}"
GEMFIRE_LINK_DIRECTORY="/opt/pivotal/gemfire/Pivotal_GemFire"

NATIVE_CLIENT_FILENAME="pivotal-gemfire-native-${NATIVE_CLIENT_VERSION}-build.10-Linux-64bit.tar.gz"
NATIVE_CLIENT_DIRECTORY="/opt/pivotal/gemfire/pivotal-gemfire-native"
NATIVE_LINK_DIRECTORY="/opt/pivotal/gemfire/NativeClient"

set -e

echo "Setting up Linux"

INSTALL_CHK=`ruby -v | grep "ruby 2.4" | wc -l`
if [ "$INSTALL_CHK" != "1" ]; then
apt-add-repository -y ppa:openjdk-r/ppa
apt-add-repository -y ppa:brightbox/ruby-ng

apt-get update

apt-get -y install \
  openjdk-8-jdk \
  g++ \
  gdb \
  git \
  google-perftools \
  libgoogle-perftools-dev \
  libgtest-dev \
  htop \
  libyaml-dev \
  man \
  libssl-dev \
  sqlite3 \
  libsqlite3-dev \
  unzip \
  valgrind \
  wget \
  git-core \
  curl \
  zlib1g-dev \
  build-essential \
  libssl-dev \
  libreadline-dev \
  libxml2-dev \
  libxslt1-dev \
  libcurl4-openssl-dev \
  python-software-properties \
  libffi-dev \
  rbenv \
  ruby2.4 \
  ruby2.4-dev
fi

NVM=`nvm list 8 | grep v8.11.3 | wc -l`
if [ "$NVM" != "1" ]; then
   \curl -sL https://deb.nodesource.com/setup_8.x | bash -
   apt-get install -y nodejs
fi 



if [ ! -d $GEMFIRE_DIRECTORY ]; then
  if [ ! -e /vagrant/tmp/$GEMFIRE_SERVER_FILENAME ]; then
    echo "----------------------------------------------------"
    echo "Please download $GEMFIRE_SERVER_FILENAME"
    echo "from https://network.pivotal.io/products/pivotal-gemfire"
    echo "(Pivotal GemFire v${GEMFIRE_VERSION})"
    echo "and place it in the ./tmp subdirectory of node-gemfire."
    echo "Then re-run \`vagrant provision\`."
    echo "----------------------------------------------------"
    exit 1
  fi
  mkdir -p /opt/pivotal/gemfire > /dev/null 2>&1

  cd /opt/pivotal/gemfire
  unzip /vagrant/tmp/$GEMFIRE_SERVER_FILENAME
fi

if [ ! -e $NATIVE_CLIENT_DIRECTORY ]; then
  if [ ! -e /vagrant/tmp/$NATIVE_CLIENT_FILENAME ]; then
    echo "----------------------------------------------------"
    echo "Please download $NATIVE_CLIENT_FILENAME"
    echo "from https://network.pivotal.io/products/pivotal-gemfire"
    echo "(Pivotal GemFire Native Client Linux 64bit v${NATIVE_CLIENT_VERSION})"
    echo "and place it in the ./tmp subdirectory of node-gemfire."
    echo "Then re-run \`vagrant provision\`."
    echo "----------------------------------------------------"
    exit 1
  fi
  cd /opt/pivotal/gemfire
  tar zxvf /vagrant/tmp/$NATIVE_CLIENT_FILENAME
  chmod +x ${GEMFIRE_DIRECTORY}/bin/*
fi

sh -c "cat > /etc/profile.d/gfcpp.sh" <<EOF
if [ ! -e $NATIVE_LINK_DIRECTORY ]; then
  ln -sf $NATIVE_CLIENT_DIRECTORY $NATIVE_LINK_DIRECTORY
fi
if [ ! -e $GEMFIRE_LINK_DIRECTORY ]; then
  ln -sf $GEMFIRE_DIRECTORY $GEMFIRE_LINK_DIRECTORY
fi
export GFCPP=$NATIVE_LINK_DIRECTORY
export GEMFIRE=$GEMFIRE_LINK_DIRECTORY
EOF
sh -c "cat >> /etc/profile.d/gfcpp.sh" <<'EOF'
export JAVA_HOME=$(readlink -f /usr/bin/javac | sed "s:/bin/javac::")
export PATH=$GEMFIRE/bin:$GFCPP/bin:/usr/local/bin:$PATH
export LD_LIBRARY_PATH=$GFCPP/lib:$LD_LIBRARY_PATH
EOF

wget --no-verbose https://raw.githubusercontent.com/google/styleguide/gh-pages/cpplint/cpplint.py -O /usr/local/bin/cpplint.py
chmod +x /usr/local/bin/cpplint.py

if [ ! -e /vagrant/tmp/gppfs-0.2 ]; then
  if [ ! -e /vagrant/tmp/gppfs-0.2.tar.bz2 ]; then
    wget --no-verbose -O /vagrant/tmp/gppfs-0.2.tar.bz2 http://www.joachim-reichel.de/software/gppfs/gppfs-0.2.tar.bz2
  fi
  cd /vagrant/tmp
  tar jxf gppfs-0.2.tar.bz2
fi
