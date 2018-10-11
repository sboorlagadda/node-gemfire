# -*- mode: ruby -*-
# vi: set ft=ruby :

# Vagrantfile API/syntax version. Don't touch unless you know what you're doing!
VAGRANTFILE_API_VERSION = "2"

Vagrant.configure(VAGRANTFILE_API_VERSION) do |config|
  # All Vagrant configuration is done here. The most common configuration
  # options are documented and commented below. For a complete reference,
  # please see the online documentation at vagrantup.com.
  config.vm.box = "ubuntu/trusty64"
  config.vm.hostname = "node-gemfire-dev"
  config.vm.define "node-gemfire-dev"

  # Create a forwarded port mapping which allows access to a specific port
  # within the machine from a port on the host machine. In the example below,
  # accessing "localhost:8080" will access port 80 on the guest machine.
  # config.vm.network "forwarded_port", guest: 80, host: 8080
  config.vm.network "forwarded_port", guest: 7070, host: 7070, auto_correct: true
  config.vm.network "forwarded_port", guest: 8080, host: 8080, auto_correct: true

  # View the documentation for the provider you're using for more
  # information on available options.
  config.vm.provider "virtualbox" do |vb|
    vb.name = "node-gemfire-dev"
    vb.memory = 4096
    vb.cpus = 2
  end

  config.ssh.forward_agent = true

  config.vm.provision "shell", path: "bin/vagrant_setup_linux.sh"
  config.vm.provision "shell", privileged: false, path: "bin/vagrant_node.sh"
  config.vm.provision "shell", privileged: false, path: "bin/vagrant_gdb.sh"
  config.vm.provision "shell", privileged: false, path: "bin/vagrant_build_project.sh"
end
