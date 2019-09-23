#!/usr/bin/env python2

import json
import urllib2
import pprint
from time import sleep

import boto3
import argparse

import sys
import ssl

""" Config here: """
synonyms = {'aliases': {},
            'groups': [['ulpm', 'ultra low power mode'],
                       ['AirVantage Connector', 'avc'],
                       ['cfg', 'config'],
                       ['cellnet', 'cellular network'],
                       ['fd', 'file descriptor'],
                       ['GNSS', 'Global Navigation Satellite System'],
                       ['ips', 'Input Power Supply'],
                       ['mcc', 'modem call control'],
                       ['Modem Data Control', 'mdc'],
                       ['fwupdate', 'Firmware Update'],
                       ['Modem Radio Control', 'mrc'],
                       ['pm', 'power manager'],
                       ['position', 'pos', 'positioning'],
                       ['secstore', 'secure storage'],
                       ['rtc', 'real time clock', 'realtime clock'],
                       ['wdog', 'watchdog'],
                       ['sup', 'supervisor'],
                       ['adc', 'Analog to Digital Converter']]}

# url parameters that the api forwards to the backend
api_url_parameters = [
    'return', 'bq', 'sort', 'fq', 'q', 'size'
]

# useful terminal colours
class bcolors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'


parser = argparse.ArgumentParser(
    description='Create a CloudSearch domain for Legato docs, and route a CORS proxy API Gateway to it.')

parser.add_argument("domain", type=str, help="Search domain to upload to.")
parser.add_argument("-c", "--config", metavar="config.json", dest="cfg", default='config.json',
                    help="Location of AWS credentials. (default: config.json)")
parser.add_argument("-r", "--region", dest="region", default='us-west-2',
                    help="AWS region (default: us-west-2)")

args = parser.parse_args()
domain = args.domain
region = args.region
cfg = json.load(open(args.cfg))

print(bcolors.BOLD + "1. CloudSearch Setup" + bcolors.ENDC)
client = boto3.client(
    'cloudsearch',
    aws_access_key_id=str(cfg['aws_access_key_id']),
    aws_secret_access_key=str(cfg['aws_secret_access_key']),
    region_name='us-west-2'
)

print("Creating domain %s ..." % domain)
response = client.create_domain(
    DomainName=domain
)

our_ip = urllib2.urlopen('https://api.ipify.org').read()
if our_ip:
    print("Our IP is " + our_ip)
else:
    print("Couldn't determine external IP in order to configure access policies.")
    our_ip = raw_input("Enter IP that'll be able to upload documents: ")

print("Setting access policy ...")
client.update_service_access_policies(
    DomainName=domain,
    AccessPolicies=json.dumps(
        {"Version": "2008-10-17",
         "Statement":
             [
                 {
                     "Effect": "Allow",
                     "Principal": {
                         "AWS": "*"
                     },
                     "Action": [
                         "cloudsearch:search",
                         "cloudsearch:suggest"
                     ]
                 },
                 {
                     "Sid": "upload_only",
                     "Effect": "Allow",
                     "Principal": "*",
                     "Action": "cloudsearch:document",
                     "Condition": {
                         "IpAddress": {
                             "aws:SourceIp": our_ip
                         }
                     }
                 }
             ]
         })
)

print("Setting analysis scheme (synonyms) ...")

scheme = {'AnalysisSchemeName': 'synonyms',
          'AnalysisOptions': {'AlgorithmicStemming': 'none', 'StemmingDictionary': '{}',
                              'Synonyms': json.dumps(synonyms), 'Stopwords': '[]',
                              'JapaneseTokenizationDictionary': ''}, 'AnalysisSchemeLanguage': 'en'}

client.define_analysis_scheme(
    DomainName=domain,
    AnalysisScheme=scheme
)

print("Defining index fields ...")
for i in ['category', 'content', 'context', 'id', 'title']:
    text_opts = {
        'DefaultValue': '',
        'ReturnEnabled': True,
        'SortEnabled': True,
        'HighlightEnabled': True
    }
    # apply synonym scheme only to these fields:
    if i in ['title', 'content']:
        text_opts['AnalysisScheme'] = 'synonyms'
    print("  + %s" % i)
    client.define_index_field(DomainName=domain,
                              IndexField={
                                  'IndexFieldName': i,
                                  'IndexFieldType': 'text',
                                  'TextOptions': text_opts,
                              })

print("Initiating document indexing ...")
client.index_documents(DomainName=domain)

print("Getting search endpoint ...")


def get_endpoint():
    lst = client.describe_domains(DomainNames=[domain])['DomainStatusList']
    if lst:
        return lst[0]['SearchService'].get('Endpoint', None)
    return None


search_endpoint = get_endpoint()
if search_endpoint is None:
    print("Waiting for domain endpoint to become available...")
    print("This can take 10 minutes for new domains.")
    check_countdown = 60
    while search_endpoint is None:
        if check_countdown <= 0:
            sys.stdout.write('\r  -> Checking...      \r')
            sys.stdout.flush()
            search_endpoint = get_endpoint()
            check_countdown = 15
            continue
        sys.stdout.write('  -> Next check in %ss.   \r' % check_countdown)
        sys.stdout.flush()
        check_countdown -= 1
        sleep(1)
    print("  -> Got it!        ")
print("  -> " + search_endpoint)

print(bcolors.BOLD + "2. API Gateway Setup" + bcolors.ENDC)

proxy_name = domain + " proxy"

client = boto3.client(
    'apigateway',
    aws_access_key_id=str(cfg['aws_access_key_id']),
    aws_secret_access_key=str(cfg['aws_secret_access_key']),
    region_name='us-west-2'
)

print("Checking if API with that name already exists ...")
apis = client.get_rest_apis()['items']

existing_id = next((item['id'] for item in apis if item['name'] == proxy_name), None)

if existing_id:
    print("  -> Deleting %s ..." % existing_id)
    client.delete_rest_api(restApiId=existing_id)
print("Creating API '%s' ..." % proxy_name)
response = client.create_rest_api(
    name=proxy_name,
    description='CORS Proxy for ' + domain
)
api_id = response['id']
print("Getting root resource ...")
response = client.get_resources(
    restApiId=api_id
)
root_id = response['items'][0]['id']
print("  = %s" % root_id)
print("Creating /GET method ...")
client.put_method(
    restApiId=api_id,
    resourceId=root_id,
    httpMethod='GET',
    authorizationType='NONE',
    apiKeyRequired=False,
    requestParameters={
        ('method.request.querystring.' + p): False for p in api_url_parameters
        }
)

print("Creating integration ...")
client.put_integration(
    restApiId=api_id,
    resourceId=root_id,
    httpMethod='GET',
    type='HTTP',
    integrationHttpMethod='GET',
    uri='https://%s/2013-01-01/search' % search_endpoint,
    requestParameters={
        ('integration.request.querystring.' + p): ('method.request.querystring.' + p) for p in api_url_parameters
        }
)

print("Creating method response ...")
client.put_method_response(
    restApiId=api_id,
    resourceId=root_id,
    httpMethod='GET',
    statusCode='200',
    responseModels={'application/json': 'Empty'},
    responseParameters={'method.response.header.Access-Control-Allow-Origin': False}
)
print("Creating integration response ...")
client.put_integration_response(
    restApiId=api_id,
    resourceId=root_id,
    httpMethod='GET',
    statusCode='200',
    responseParameters={'method.response.header.Access-Control-Allow-Origin': "'*'"},
    responseTemplates={'application/json': ''}
)
print("Deploying API ...")
client.create_deployment(
    restApiId=api_id,
    stageName='prod',
)

api_endpoint = 'https://%s.execute-api.%s.amazonaws.com/prod' % (api_id, region)

print(bcolors.BOLD +  "Important: Set invoke_url in main.js to this endpoint:"  + bcolors.ENDC)
print('    ' + bcolors.OKGREEN + api_endpoint + bcolors.ENDC)

print(bcolors.BOLD + "3. Test" + bcolors.ENDC)

print("Testing API endpoint for valid JSON ...")


print("  -> " + bcolors.OKBLUE +  "GET " + bcolors.ENDC + api_endpoint)

try:
    test_data = urllib2.urlopen(api_endpoint).read()
    json_data = json.loads(test_data)
    if 'status' in json_data and 'hits' in json_data:
        print(bcolors.OKGREEN + bcolors.BOLD + "Looks good!" + bcolors.ENDC)
    else:
        print(bcolors.WARNING + "Unexpected JSON format:" + bcolors.ENDC)
        pprint.pprint(json_data)
except urllib2.URLError, e:
    print(bcolors.FAIL + "Couldn't fetch data from API!!" + bcolors.ENDC)
    print(e)
except ValueError, e:
    print(bcolors.FAIL + "Seems to be invalid JSON!!" + bcolors.ENDC)
    print(e)
    print(test_data)
else:
    print(bcolors.OKBLUE + "Done." + bcolors.ENDC)
