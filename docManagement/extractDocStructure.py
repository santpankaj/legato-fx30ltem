#!/usr/bin/env python2

from bs4 import BeautifulSoup
import xmltodict
import json
from argparse import ArgumentParser
from uuid import uuid4
import sys
import os




def generate_tree_IDs_helper(tree, id = None):
    """generate a unique ID for each node. This is needed for saving tree state on the page
    also contains a workaround for a single child becoming a
    json object rather than an array (which is needed by jqtree)
    """
    if id is None:
        id = 0
    tree["id"] = id
    id += 1
    if "children" in tree:
        kids = tree["children"]
        if isinstance(kids, list):
            for t in kids:
                id = generate_tree_IDs_helper(t, id)
        elif isinstance(kids, dict):
            tree["children"] = [tree["children"]] # workaround that converts children to a list
            id = generate_tree_IDs_helper(kids, id)
        else:
            raise Exception("children is neither a list nor a dictionary. What???")
    return id

def generate_tree_IDs(tree):
    generate_tree_IDs_helper(tree['toc'])

def move_node(what, whence, to):
    """move tree node 'what' from 'whence' to 'to'
    that is, change the parent of 'what' to 'to'
    """
    whence['children'].remove(what)
    to['children'].append(what)


# (safely) find a child with a certain label
def find_node(label, where):
    opts = [x for x in where['children'] if x['@label'] == label]
    if len(opts) != 1:
        print("-!!!PROBLEM!!!!-")
        print("I need a single '%s' node under '%s' but I " % (label, where['@label']) + \
              ("found %s such nodes!" % len(opts) if opts else "couldn't find it."))
        print("Here's what I was looking through:")
        print(where['@label'] + ":")
        for x in where['children']:
            print("    " + x['@label'])
        sys.exit(1)
    return opts[0]


'''convert extracted xml data to json data in filename.json file'''
def convertXMLtoJson(xml, moveref):
    dict_obj = xmltodict.parse(xml) #parse xml obj to dict obj

    if moveref:
        ##################  MOVE REFERENCE UNDER API ##############################
        print("Moving node 'Reference' to 'Build Apps/API Guides/C Prototypes'...")
        ref = find_node('Reference', dict_obj['toc'])
        ref['@label'] = "C Prototypes" # rename
        bapp = find_node('Build Apps', dict_obj['toc'])
        apiguide = find_node('API Guides', bapp)
        move_node(ref, dict_obj['toc'], apiguide)
        print("Moved.")

    generate_tree_IDs(dict_obj)
    return json.dumps(dict_obj)


if __name__ == '__main__':
    parser = ArgumentParser(description="Convert toc.xml into a json file, assigning each node a unique ID.")
    parser.add_argument('filename')
    parser.add_argument("-o", "--output", dest="output", metavar="FILE",
                      help="json file to write. Default: same name as input, with .json extension instead of .xml")
    parser.add_argument('--move-reference', dest="moveref", action="store_true", help='Move node' +
                        ' "Reference" under "C Prototypes"')
    args = parser.parse_args()

    outfilename = args.output
    filename = args.filename

    if outfilename is None:
        outfilename = os.path.splitext(filename)[0] + '.json'


    print "converting " + filename + " -> " + outfilename
    try:
        with open(filename) as file:
            soup = BeautifulSoup(file, "html.parser")    #toc.xml from doxygen, store documentation structure

        toc_root = soup.find("toc")

        # rename topic tags to children (for navigation tree)
        for topic in toc_root.find_all('topic'):
            topic.name='children'
        json_content=convertXMLtoJson(toc_root.encode(), args.moveref)
        new_string=json_content.replace("@","")
        '''
        function: generate a json file
        '''
        with open(outfilename,'w') as f:
            f.write(new_string)
    except IOError as e:
        print e.message

