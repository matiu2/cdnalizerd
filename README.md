# CDNAlizerD

Watches directories and synchronizes changes to the file system to Rackspace Cloud Files. Like lsyncd but lsyncs to cloud files.

**Development currently on hold**

I don't think there's enough demand for this. If wish that it existed, let me know via github message (matiu2). If I get any interest at all, I'll start working on it again.

## Config file

The default config file location is /etc/cdnalizerd.conf

Each line in the config file is either:

 * a comment - eg. # Comment text
 * a setting - username=letmein
 * a path pair - /var/www/vhosts/mysite.com/static /

### Settings

These are the available settings:

 * username=hello - The Rackspace/Nova API username to log in with
 * apikey=1234 - The API key
 * region=SYD - The region to log in to (ORD/DFW/SYD/HKG)
 * container=mysite-images - The container to sync with
 * snet=true - Use the datacenter service net to log in. Set to true if your cloud files region is in the same datacenter that you're running cdnalizerd in. Defaults to false

Each setting line affects all lines below it, until it's overridden.

### Path line

Once you have entered the settings, you can enter a pair of paths, the first in the pair is the local file system path. The second is the sub directory for the container. It is affected by all the settings above it

Settings aren't processed until we hit a path line. The second setting always replaces the first.

### Example

Line numbers were added for clarity; don't put them into production files.

    1:  # My login
    2:  username=hello
    3:  apikey=1234
    4: 
    5:  # Use DFW over the servicenet
    6:  snet=true
    7:  region=DFW
    8: 
    9:  # mysite1.com
    10: container=mysite1.com
    11: /var/www/vhosts/mysite1.com/static/images /images
    12: /var/www/vhosts/mysite1.com/static/css /css
    13: 
    14: # mysites2.com
    15: container=mysite2.com
    16: /var/www/vhosts/mysite2.com/static /
    17: 
    18: # Australian site
    19: snet=false
    20: region=SYD
    21: container=mysite.com.au
    22: /var/www/vhosts/mysite.com.au/static/images /images

So in the above example, username and api key are used for all path pairs. On line 14 container is overridden for subsequent path pair entries.
On lines 19, 20, and 21 snet, region and container are overriden for all subsequent path pairs.
