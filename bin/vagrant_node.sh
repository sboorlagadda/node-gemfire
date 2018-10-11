if ! [ -e ~/.nvm/nvm.sh ]; then
  curl -o- https://raw.githubusercontent.com/creationix/nvm/v0.33.11/install.sh | bash
fi
source ~/.nvm/nvm.sh

echo "***********************************************************"
echo "****** Validating NVM/NPM/Grunt/Jasmine installation ******"
echo "***********************************************************"

GRUNT=`nvm exec which grunt | grep grunt | wc -l`
if [ "$GRUNT" != "1" ]; then
  echo "******************************************"
  echo "****** Installing Grunt and Jasmine ******"
  echo "******************************************"
  nvm install v8
  nvm exec 8 npm install -g grunt jasmine
fi

#nvm list 8 || nvm install -s 8 || npm install -g npm@6
#nvm exec 8 which grunt || nvm exec 8 npm install -g grunt jasmine
