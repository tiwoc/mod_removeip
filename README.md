# Readme

This is a version of mod_removeip for Apache 2 that introduces support for IPv6 and for optional shortening of addresses. The goal is to offer better privacy to your users and to comply with data protection laws, for example in Germany. Please note, however, that I am not a lawyer and thus cannot guarantee that especially the latter goal is achieved by this piece of software.

## Build and Install

If I use *sudo* in the following, I mean *become root*, either by prepending *sudo*, by getting a root shell via *su* or through other means.

### Debian and derivatives (Ubuntu, Mint, ...), using dpkg

Offering pre-built packages is in the works, probably using a PPA. Until then, you need to build them yourself.

To build the .deb package, you need to have build-essential, devscripts and debhelper installed:

    $ sudo apt-get install build-essential devscripts debhelper

Then, do as follows:

    $ cd /path/to/libapache2-mod-removeip_xyz/
    $ debuild -us -uc
    $ cd ..
    $ sudo dpkg -i libapache2-mod-removeip_xyz.deb
    $ sudo a2enmod removeip
    $ sudo service apache2 restart

Now that you're done, please skip to Configuration.

### Other operating systems, using make

First, you need to install the Apache development files, especially the development headers and the apxs2 utility, probably using your package manager. Then do as usual:

    $ make
    $ sudo make install

Now, you need to manually add lines to your Apache configuration that load the module and enable it (see removeip.{load,conf}). Finally, you need to restart the Apache.


## Configuration

You can either choose to completely replace the IP addresses of you users, or to mask/shorten them so that a single user cannot be tracked, but basic web analytics are still possible. When running on Debian or a derivative and installing using dpkg as explained above, basic configuration can be found in /etc/apache2/mods-available/removeip.conf.

### Completely replace addresses
    # put this in global server config; won't work on VirtualHost or Directory level
    RemoveIpEnable On

#### Results (examples)
209.85.148.138 -> 127.0.0.1  
2a00:1450:4001:c01::8b -> ::1


### Mask addresses
    # put this in global server config; won't work on VirtualHost or Directory level
    RemoveIpEnable On
    RemoveIpShortenOnly On

#### Results (examples)
209.85.148.138 -> 209.85.0.1  
2a00:1450:4001:c01::8b -> 2a00:1450::1

Here, addresses are shortened to the first 2 (IPv4) or 4 (IPv6) bytes which should be enough to extract the country and ISP of the user.
