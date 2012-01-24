"""
Hudson specific utility classes
"""
import urllib, urllib2, os, time

BUILD_JOB    = 'job/%(name)s/build'
BUILD_WITH_PARAMS_JOB = 'job/%(name)s/buildWithParameters'

class HudsonError(Exception):
    def __init__(self, value):
        self.value = value
    def __str__(self):
        return repr(self.value)   

"""
Start the given job with the given parameters
"""
def startJob( root_url, jobname, jobparams = None ):
    joburl = ''
    if jobparams:
        joburl = root_url + '/' + BUILD_WITH_PARAMS_JOB%{'name':jobname} + '?' + urllib.urlencode(jobparams)
    else:
        joburl = root_url + '/' + BUILD_JOB%{'name':jobname}

    req = urllib2.Request(joburl)
    # we need to disable the environment set proxies
    proxy_support = urllib2.ProxyHandler({})
    opener = urllib2.build_opener(proxy_support)
    urllib2.install_opener(opener)
    
    try:
        urllib2.urlopen(req).read()
    except urllib2.HTTPError, e:
        # Jenkins's funky authentication means its nigh impossible to distinguish errors.
        if e.code in [401, 403, 500]:
            raise HudsonError('Error in request to start Jenkins job. Possibly authentication failed [%s]'%(e.code))
        else:
            raise HudsonError('Error in request to start Jenkins job. Error [%s].'%(e.code) )
        # right now I'm getting 302 infinites on a successful delete

"""
Retrieves Hudson Job information
"""
def getDownstreamJobNumber(upstream_project, downstream_project, upstream_build_number = ''):
    count = 1
    max_count = 5
    time_out = 30
    s = 0
    jenkins_url = os.environ["JENKINS_URL"]
    
    if( '' == upstream_build_number):
        upstream_build_number = os.environ["BUILD_NUMBER"]
        
    full_url = jenkins_url + '/job/' + downstream_project + '/api/xml?depth=1&xpath=/matrixProject/build[action/cause/upstreamProject="' + upstream_project + '" and action/cause/upstreamBuild="' + upstream_build_number + '"]/number/text()'
    print 'Retrieving the latest build number from:' + full_url
    
    while count < max_count:
        hf = urllib.urlopen(full_url , proxies={})
        hr = hf.getcode()
        if 200 == hr:
            s = hf.read()
            hf.close()
            break
        else:
            count += 1
            print 'Error Response Code: ' + str(hr) + '. Retrying (' + str(count) + '/' + str(max_count) + ')'
            time.sleep(time_out)

    if max_count == count:
        raise HudsonError("The access to the Jenkins site has been lost:")

    print 'Build Number retrieved = ' + s
    return s
        
class ParametersFile:
    """
    Writes the parameter file in 'copy artifact' plug-in format
    """
    def __init__(self, filename):
        self.filename = filename
        self.params = {}
       
    def addParam(self, name, value):
        self.params[name] = value
        
    def writeFile(self):
        f = open(self.filename, 'w')
        for (k, v) in self.params.items():
            f.write('\n' + k + '=' + v)
        f.close()
