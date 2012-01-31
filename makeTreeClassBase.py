#!/usr/bin/env python
#______________________________________________________________
# makeTreeClassBase.py
# 
# This should be called within the macros/ directory, giving
# a rootfile containing the desired version of the Ntuples as
# an argument
#______________________________________________________________


import sys, subprocess, os,re
#from ROOT import TTree, TFile, gDirectory

usage = "Usage: makeTreeClassBase.py filename.root"

if len(sys.argv) < 2:
    print usage
    exit(1)

FILENAME = sys.argv[1]
CLASSNAME = 'TreeClassBase'
HEADERNAME = CLASSNAME + '.hh'
SOURCENAME = CLASSNAME + '.cc'

#______________________________________________________________
def isVector(typename):
    return (typename[-1] == 's')

#______________________________________________________________
def typename(vartype):
    typename = vartype.lower() # protect against "Strings"
    # Special case for strings
    if typename.find('string')>-1:
        typename = 'std::string'

    if isVector(vartype): return 'std::vector<'+typename.rstrip('s')+'>'
    else: return typename
        
#______________________________________________________________
def declareVars(names,file):
    indent = 35 # Formatting attempt
    for k,v in sorted(names.iteritems()):
        type = typename(v)
        # Variable
        spaces = (indent-len(type))*' '+' '
        file.write(4*' '+type+spaces+' '+k+';\n')
        # edm::Handle
        spaces = (indent-14-len(type))*' '+' '
        file.write(4*' '+'edm::Handle<'+type)
        if isVector(v): file.write(' >'+spaces+'h'+k+';\n')
        else: file.write('> '+spaces+'h'+k+';\n')
        # edm::InputTag
        spaces = (indent-13)*' '+' '
        file.write(4*' '+'edm::InputTag'+spaces+'t'+k+';\n')

#______________________________________________________________
def getVars(names,file,spaces,treename):
    indent = spaces*' '
    for k,v in sorted(names.iteritems()):
        line = 'result &= '+treename+'getByLabel( t'+k+', h'+k+' );\n'
        file.write(indent+line)
        line = 'if ( h'+k+'.isValid() ) '+k+' = *h'+k+';\n'
        file.write(indent+line)
        

#______________________________________________________________
def defineLabels(names,file):
    indent = 25
    for k,v in sorted(names.iteritems()):
        spaces = (indent-len(k))*' '
        file.write(4*' '+'t'+k+spaces+' = edm::InputTag("analyze","'+k+'");\n')

#______________________________________________________________
def processImpl(rBranches,eBranches):
    finput = open('src/base/'+SOURCENAME+'.tpl')
    foutput = open(SOURCENAME,'w')
    patGet    = ('\s*<GET(\w+)HANDLES>.*')
    patLabels = ('\s*<DEFINELABELS>.*')

    for line in finput.readlines():
        m1 = re.match(patGet,line)
        m2 = re.match(patLabels,line)
        if m1:
            if m1.group(1) == 'RUN':
                getVars(rBranches,foutput,6,'run.')
            elif m1.group(1) == 'EVENT':
                getVars(eBranches,foutput,4,'event->')
            else:
                print >>sys.stderr,'*** Unknown header pattern:',line
        elif m2:
            defineLabels(rBranches,foutput)
            defineLabels(eBranches,foutput)
        else:
            foutput.write(line)

    finput.close()
    foutput.close()
    return foutput.name

#______________________________________________________________
def processHeader(rBranches,eBranches):
    finput = open('include/base/'+HEADERNAME+'.tpl')
    foutput = open(HEADERNAME,'w')
    pattern = ('\s*<(\w+)LEAFDECLARATION>.*')

    for line in finput.readlines():
        m = re.match(pattern,line)
        if m:
            if m.group(1) == 'RUN':
                declareVars(rBranches,foutput)
            elif m.group(1) == 'EVENT':
                declareVars(eBranches,foutput)
            else:
                print >>sys.stderr,'*** Unknown header pattern:',line
        else:
            foutput.write(line)

    finput.close()
    foutput.close()
    return foutput.name
            
    
    
#______________________________________________________________
def getBranches(file,tree):
    cmd = ['edmFileUtil','-P','-t',tree,file]
    run = subprocess.Popen(cmd,stdout=subprocess.PIPE)
    output = run.communicate()[0]
    if run.returncode:
        print >>sys.stderr,"*** Error while parsing file:",output
        return []
    branches = dict()
    pattern = re.compile(r".*?(\w+)_analyze_(\S+)_NTupleProducer.*")
    for line in output.split('\n'):
        m = re.match(pattern,line)
        if m:
            branches[m.group(2)] = m.group(1)

    return branches

#______________________________________________________________
if __name__=='__main__':

#     # Check location
#     if os.getcwd().find('macros') < 0:
#         print 'This script has to be run in the \'macros\' directory'
#         print 'where you wish to update the TreeClassBase class'
#         exit(2)

    # Check input file
    if not os.path.exists(FILENAME):
        print 'File',FILENAME,'does not seem to exist'
        exit(3)
    
    print 'Processing input file...'
    runBranches = getBranches(FILENAME,'Runs')
    eventBranches = getBranches(FILENAME,'Events')
    print '  -> Found',len(runBranches),'run branches',
    print 'and',len(eventBranches),'event branches'

    # Process templates and write to output files
    hName = processHeader(runBranches,eventBranches)
    print '  -> Wrote',hName
    iName = processImpl(runBranches,eventBranches)
    print '  -> Wrote',iName
	
    subprocess.call(['mv', '-vf', hName, 'include/base/'])
    subprocess.call(['mv', '-vf', iName, 'src/base/'])
