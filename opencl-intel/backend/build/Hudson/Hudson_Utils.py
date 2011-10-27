"""
Hudson specific utility classes
"""
import urllib,os,time

"""
Retrieves Hudson Job information
"""
def getDownstreamJobNumber(self, upstream_project, downstream_project, upstream_build_number = ''):
    count = 1
    max_count = 5
    time_out = 5
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
        raise Exception("The access to the Jenkins site has been lost:")

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



#--------------------------------

