#!/bin/sh

set -e

echo "*********************************************************"
echo "****** Validating Node GemFire/Ruby Bundler/Bundle ******"
echo "*********************************************************"

cd /vagrant

if [ ! -d "lib/binding/Release/node-v57-linux-x64" ]; then
  echo "*************************************"
  echo "****** Installing Node GemFire ******"
  echo "*************************************"
  npm install
fi

VERSION=`gem list | grep "bundler (1.16.2)" | wc -l`
if [ "$VERSION" != "1" ]; then 
  echo "********************************"
  echo "****** Installing Bundler ******"
  echo "********************************"
  gem install bundler
fi  
  
rbenv rehash

VERSION=`bundle check | grep "dependencies are satisfied" | wc -l`
if [ "$VERSION" != "1" ]; then
  echo "*******************************"
  echo "****** Installing bundle ******"
  echo "*******************************"
  bundle install
fi

# For script debugging
VERSION=`npm search node-inspect | grep "Node Inspect" | wc -l`
if [ "$VERSION" != "1" ]; then
  echo "*************************************"
  echo "****** Installing node-inspect ******"
  echo "*************************************"
  npm install -g node-inspect
fi

if [ ! -d "tmp/gemfire" ]; then
  mkdir tmp/gemfire
fi

echo ""
echo ""
echo "Ready to go! Run the following to get started:"
echo ""
echo "$ vagrant ssh"
echo "$ cd /vagrant"
echo "$ grunt"
