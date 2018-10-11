#!/bin/sh

set -e

echo "*********************************************************"
echo "****** Validating Node GemFire/Ruby Bundler/Bundle ******"
echo "*********************************************************"

cd /vagrant

if [ ! -d "tmp/gemfire" ]; then
  mkdir tmp/gemfire
  echo "export JAVA_HOME=/usr/lib/jvm/java-8-openjdk-amd64" >>~/.bashrc
fi


if [ ! -d "lib/binding/Release/node-v57-linux-x64" ]; then
  echo "*************************************"
  echo "****** Installing Node GemFire ******"
  echo "*************************************"
  npm install node-pre-gyp -g 
  npm install aws-sdk -g
  npm install
fi


BUNDLER=`gem list | grep "bundler (1.16.2)" | wc -l`
if [ "$BUNDLER" != "1" ]; then 
  echo "********************************"
  echo "****** Installing Bundler ******"
  echo "********************************"
  sudo gem install bundler
fi  
  
rbenv rehash

BUNDLE=`bundle check | grep "dependencies are satisfied" | wc -l`
if [ "$BUNDLE" != "1" ]; then
  echo "*******************************"
  echo "****** Installing bundle ******"
  echo "*******************************"
  sudo bundle install
fi

echo ""
echo ""
echo "Ready to go! Run the following to get started:"
echo ""
echo "$ vagrant ssh"
echo "$ cd /vagrant"
echo "$ grunt"
