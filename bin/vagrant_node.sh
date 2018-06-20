if ! [ -e ~/.nvm/nvm.sh ]; then
  curl -o- https://raw.githubusercontent.com/creationix/nvm/v0.33.11/install.sh | bash
fi
source ~/.nvm/nvm.sh

echo "***********************************************************"
echo "****** Validating NVM/NPM/Grunt/Jasmine installation ******"
echo "***********************************************************"

VERSION=`nvm list 8 | grep v8.11.3 | wc -l`
if [ "$VERSION" != "1" ]; then
  echo "****************************"
  echo "****** Installing NVM ******"
  echo "****************************"
  nvm install -s 8 
fi 

VERSION=`npm version | grep "npm: '6.1.0'" | wc -l`
if [ "$VERSION" != "1" ]; then
  echo "****************************"
  echo "****** Installing NPM ******"
  echo "****************************"
  npm install -g npm@6
fi 

VERSION=`which grunt | grep grunt | wc -l`
if [ "$VERSION" != "1" ]; then
  echo "******************************"
  echo "****** Installing Grunt ******"
  echo "******************************"
  nvm exec 8 npm install -g grunt
fi

VERSION=`which jasmine | grep jasmine | wc -l`
if [ "$VERSION" != "1" ]; then
  echo "********************************"
  echo "****** Installing Jasmine ******"
  echo "********************************"
  nvm exec 8 npm install -g jasmine
fi

#nvm list 8 || nvm install -s 8 || npm install -g npm@6
#nvm exec 8 which grunt || nvm exec 8 npm install -g grunt jasmine

nvm alias default 8
