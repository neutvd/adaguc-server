"""
 This Python script runs the ADAGUC executable without an webserver. It can be used as example to run ADAGUC in your own environment from python.
 Created by Maarten Plieger - 20151104
"""

#URL without cascaded layers, for performance testing:
url="source=testdata.nc&SERVICE=WMS&SERVICE=WMS&VERSION=1.3.0&REQUEST=GetMap&LAYERS=testdata&WIDTH=256&HEIGHT=256&CRS=EPSG%3A4326&BBOX=30,-30,75,30&STYLES=testdata%2Fnearest&FORMAT=image/png&TRANSPARENT=FALSE&"

#This is the url for the ADAGUCServer used to create maps, 
#with baselayer and overlay coming from http://geoservices.knmi.nl/ as configured in DefaultLayers.include.xml 
#  Use this URL instead to see pretty visualization 

#url="source=testdata.nc&SERVICE=WMS&SERVICE=WMS&VERSION=1.3.0&REQUEST=GetMap&LAYERS=baselayer,testdata,overlay&WIDTH=1000&HEIGHT=750&CRS=EPSG%3A4326&BBOX=30,-30,75,30&STYLES=testdata%2Fnearest&FORMAT=image/png&TRANSPARENT=TRUE&showlegend=true&title=test&subtitle=test&showdims=true&showscalebar=true&shownortharrow=true"

import subprocess
import os
from os.path import expanduser
from PIL import Image
from StringIO import StringIO
from adaguc.CGIRunner import CGIRunner
import io
import tempfile
import shutil

""" ADAGUC_TMP is the location where logfiles are stored. 
  In current config file adaguc.autoresource.xml, the DB is written to this temporary directory. 
  Please note regenerating the DB each time for each request can cause performance problems. 
  You can safely configure a permanent location for the database which is permanent in adaguc.autoresource.xml (or your own config)"""
ADAGUC_TMP = tempfile.mkdtemp()

# ADAGUC_PATH is the location of the adagucserver repository.
ADAGUC_PATH=expanduser("~")+"/adagucserver/"

""" Setup a new environment """
adagucenv=os.environ.copy()

""" Set required environment variables """
adagucenv['ADAGUC_PATH']=ADAGUC_PATH
adagucenv['ADAGUC_TMP']=ADAGUC_TMP
adagucenv['ADAGUC_CONFIG']=ADAGUC_PATH+"/data/config/adaguc.autoresource.xml"
adagucenv['ADAGUC_DATARESTRICTION=']="FALSE" 
adagucenv['ADAGUC_LOGFILE']=ADAGUC_TMP+"/adaguc.autoresource.log"
adagucenv['ADAGUC_FONT']=ADAGUC_PATH+"/data/fonts/FreeSans.ttf"
adagucenv['ADAGUC_ONLINERESOURCE']=""

""" Run the ADAGUC executable and capture the output """
filetogenerate =  StringIO()
status = CGIRunner().run(["../../bin/adagucserver"],url,output = filetogenerate,extraenv=adagucenv)

try:
  """ Try to show the image """
  img = Image.open(StringIO(filetogenerate.getvalue()))
  img.show()
except:
  """ Otherwise print the logfile, giving information on what went wrong """
  print "Unable to run ADAGUC WMS, listing logs:"
  f=open(ADAGUC_TMP+"/adaguc.autoresource.log")
  print f.read()
  f.close()
  pass

""" Remove the temporary directory """
shutil.rmtree(ADAGUC_TMP)