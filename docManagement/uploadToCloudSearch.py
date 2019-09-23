#!/usr/bin/env python2

import argparse
import json
import os
import pprint
import sys
import threading
import hashlib
import boto3
import random
import time
from bs4 import BeautifulSoup

from tqdm import tqdm

# how many documents to try searching at the end
test_count = 5

def read_config(loc):
    with open(loc) as f:
        return json.load(f)


def populate_dict_with_cats(node, maxdepth, path_to_parent, dic, depth):
    """A recursive function to take a node and its children, and for each file get its 'context'
    (so on a Build Apps page, you only search on Build Apps, etc)
    and breadcrumb path that appears in the search results.
    Example:
    dic['le__wdog__interface_8h.html'] = (u'Build Apps', u'API Guides > C Prototypes')
    """

    delimiter = " > "

    subtopics = node.get('children', [])
    filename = node['href'].split("#")[0]  # ignore everything after hash

    if depth < maxdepth and not node["label"].endswith(".h"):
        # add current node's label onto the path, with the delimiter if path isn't empty
        path_to_this_node = path_to_parent + (delimiter if path_to_parent else '') + node["label"]
    else:
        path_to_this_node = path_to_parent

    splt = path_to_parent.split(delimiter)


    path_to_parent = delimiter.join(splt[-2:])

    assert len(path_to_parent.split(delimiter)) < 3

    dic[filename] = (splt[0] or path_to_this_node, path_to_parent )

    # we record path to parent instead of this node, because for example "how_to_main.html" should display
    # 'Build Apps > Concepts', not 'Build Apps > Concepts > How To' (path_to_this_node)
    # for context, if path_to_parent is empty, like for build_apps_main.html, we want to use path_to_this_node
    # just so it's searchable

    for st in subtopics:
        populate_dict_with_cats(st, maxdepth, path_to_this_node, dic, depth + 1)


def get_files_with_categories():
    """Looks through toc.json, and finds all mentioned filenames.
    Then, it assigns each file a category (getting started, api, etc), and
    returns a {filename: (category, path)} dictionary.
    """

    file_list = {}
    tocpath = os.path.join(source_path, "toc.json")
    try:
        j = json.load(open(tocpath))
    except IOError:
        print("Couldn't load %s! Make sure you've run extractDocStructure.py" % tocpath)
        sys.exit(1)
    for t in j['toc']['children']:
        populate_dict_with_cats(t, 3, '', file_list, 0)
    return file_list

def get_documents_to_delete(new_ids):
    all_docs = domain_client.search(query="matchall", queryParser="structured", size=600)
    to_delete = []
    for doc in all_docs['hits']['hit']:
        if doc['id'] not in new_ids:
            print("Marking %s for deletion. " % doc)
            to_delete.append({
                'type':'delete',
                'id':doc['id']
                })
    return to_delete

def extract_version(html):
    soup = BeautifulSoup(html,'lxml')
    try:
        return soup.find(class_='doc-version')['title']
    except:
        return None

def get_existing_version():
    resp = domain_client.search(query='"a class=\\"doc-version\\""', size=1)
    hits = resp['hits']['hit']
    if hits:
        return extract_version(hits[0]['fields']['content'][0])
    else:
        return None

def process(path, filename):
    """Read the html file, get the goodies out of it, and return it as a dict.
    """
    fullname = os.path.join(path, filename)

    file_id = filename
    # if the id is too long, hash it instead
    if len(file_id) > 127:
        file_id = hashlib.md5(file_id.encode('utf-8')).hexdigest()

    try:
        with open(fullname) as sf:
            soup = BeautifulSoup(sf, 'lxml')
    except IOError:
        return None

    soup_content = soup.find('div', {'class': 'contents'})
    ctx, cat = file_list[filename]

    content = str(soup_content)
    title = soup.find('div', {'class': 'title'}).text  # extract document title

    # this part enables a page to be found by searching "le_args" rather than "le_args.h"
    for sfx in ['.h File Reference', '_interface.h File Reference']:
        if title.endswith(sfx):
            # append title without prefix
            content += ("\n" + title[:-len(sfx)].encode()) \
                       * 3  # to give it a search rating boost over links to it
    return { 'type':'add',
    "id": file_id,
    'fields': {
        "content": content,
        "title": title,
        "category": cat,
        "context": ctx,
        "id": filename
        }
    }
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

parser = argparse.ArgumentParser(description='Upload legato docs to AWS Cloudsearch.')
parser.add_argument('-s', metavar='html_dir/', dest='source_path', type=str, default='../build/doc/user/html/',
                    help="Location of HTML files (default: ../build/doc/user/html/)")
parser.add_argument("domain_name", type=str,  help="Search domain to upload to (e.g. legato-16-04)")
parser.add_argument("--dry", dest="dry",
                    help="Dry run! Don't actually upload to AWS.", action='store_true')
parser.add_argument("-v", "--verbose", dest="verbose", action='store_true')
parser.add_argument("-c", "--config", metavar="config.json", default='config.json', dest="cfg", help="Location of AWS credentials. (default: config.json)")

args = parser.parse_args()
source_path = args.source_path
verbose = args.verbose
dry = args.dry
domain_name = args.domain_name

if dry:
    print("Heads up! I'm in --dry mode, so I won't actually delete or upload anything to AWS.")

cfg = read_config(args.cfg)  # get access_key_id and access_key from config.json

csclient = boto3.client('cloudsearch',
    aws_access_key_id=str(cfg['aws_access_key_id']),
    aws_secret_access_key=str(cfg['aws_secret_access_key']),
    region_name='us-west-2')
resp = csclient.describe_domains(DomainNames=[domain_name])['DomainStatusList']
try:
    endpoint = resp[0]['DocService']['Endpoint']
except (IndexError, KeyError):
    print("Couldn't get %s's endpoint. Response:" % domain_name)
    print(resp)
    exit(1)

domain_client = boto3.client('cloudsearchdomain',
    aws_access_key_id=str(cfg['aws_access_key_id']),
    aws_secret_access_key=str(cfg['aws_secret_access_key']),
    region_name='us-west-2',
    endpoint_url='https://' + endpoint)

oldver = get_existing_version()
if oldver:
    print("Version already up on the domain: %s" % bcolors.BOLD + oldver + bcolors.ENDC)
try:
    with open(os.path.join(source_path,"aboutLegato.html")) as f:
        newver = extract_version(f)
    if oldver:
        print("NEW version that will replace it: " + bcolors.BOLD + newver + bcolors.ENDC)
    else:
        print("Version to be uploaded: %s" % bcolors.BOLD + newver + bcolors.ENDC)
except Exception as e:
    print("Couldn't detect version of docs to be uploaded:")
    print(e)

if not dry and raw_input("Continue? (y/N) ").lower() != 'y':
    exit(1)


print("Analyzing TOC...")
file_list = get_files_with_categories()

print("Processing HTML...")
transaction=[]
new_ids=[]
duds=[]
it = (file_list if verbose else tqdm(file_list)) # use progress bar if not verbose
for filename in it:
    data = process(source_path, filename)  # dictionary with the goods
    if data:
        if verbose:
            pprint.pprint({k: v for k, v in data.iteritems() if k != 'fields'})
            pprint.pprint({'fields':{k: v for k, v in data['fields'].iteritems() if k != 'content'}}, indent=1)
        transaction.append(data)
        new_ids.append(data['id'])
    else:
        if not verbose:
            it.clear()
        print("Couldn't read %s. Skipping it." % os.path.join(source_path, filename))
        duds.append(filename)
for dud in duds:
    file_list.pop(dud, None)
print('Finding obsolete files...')
transaction.extend(get_documents_to_delete(new_ids))

if not dry:
    print("Uploading %s document operations to AWS now..." % len(file_list))
    resp = domain_client.upload_documents(documents=json.dumps(transaction),  contentType="application/json")
    if resp['status'] == 'success':
        print(bcolors.OKGREEN + bcolors.BOLD + u'\u2713 Upload OK!'+ bcolors.ENDC)
    else:
        print(resp)
        exit(1)
    print("Waiting 30 secs before testing...")
    time.sleep(30)
    for i in range(test_count):
        test_file = process(source_path, random.choice(file_list.keys()))
        query = test_file['fields']['title']
        expected_filename = test_file['fields']['id']
        print('Random search test: Searching for "%s" in hopes of finding %s...' % (query,expected_filename))
        resp = domain_client.search(query=query)
        if any(hit['fields']['id'][0] == expected_filename for hit in resp['hits']['hit']):
            print(bcolors.OKGREEN + bcolors.BOLD + u'\u2713 Test OK!'+ bcolors.ENDC)
        else:
            print(bcolors.FAIL + bcolors.BOLD + u"\u2717 Test failed.. expected %s in here:" % expected_filename + bcolors.ENDC)
            pprint.pprint(resp)
