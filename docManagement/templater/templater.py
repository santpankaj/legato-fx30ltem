#!/usr/bin/env python2

import argparse
import json
import os
import os.path
import re
import shutil
from collections import OrderedDict
from os.path import join

from bs4 import BeautifulSoup

script_dir = os.path.dirname(__file__)

parser = argparse.ArgumentParser(description='Takes the extracted content fragments and builds a functioning site.',
                                 formatter_class=argparse.ArgumentDefaultsHelpFormatter)
parser.add_argument("source_path", type=str,
                    help="Location of HTML files")
parser.add_argument("out_path", type=str,
                    help="Destination of converted HTML files", default=join(script_dir, "out/"), nargs='?')
parser.add_argument("-v", "--verbose", dest="verbose", action='store_true')

parser.add_argument('--split-top-level', action='store_true', dest='split_ctx',
                    help='Split top level categories into different independent sections of the site.')
parser.add_argument("--resource-dir", type=str,
                    help="Which directory (relative to 'resources/') to load config json files, " +
                    "static files, etc from, overriding files in 'default/'",
                    default='default')

args = parser.parse_args()

html_dir = args.source_path
src_dir = join(script_dir, "resources/", args.resource_dir)
def_src_dir = join(script_dir, "resources/", 'default/')

template_dir_name = "templates/"
json_dir_name = "json/"
static_dir_name = "static/"

out_root_dir = args.out_path

toc_dir = join(out_root_dir, "tocs/")

''' This program takes the extracted content fragments and builds a functioning site.

    Tags like %%%TOCFILE%%% are placeholders that are replaced with output of certain functions, which are mapped below
    (in the dictionary 'funcs')

    The way the templating works is when something like foo.part.html is encountered, it looks for a template called
    part.html renders any tags in it, and then replaces {content} with the content of foo.part.html,
    saving it as foo.html.

    JSON files are used for defining navigation, for example sidemenu.json and header.json define links,
    the html that those links should be formatted into, and the location of the template that the resulting block is inserted into.

    Everything in static_dir_name is copied over directly.

    A context is the highest level category (e.g. Build Apps)
    A category is lower level (e.g. Concepts)
    '''

def get_path(*args):
    '''
    Takes a path(s) relative to the resource_dir, and returns an absolute path either containing the
    directory specified by --resource-dir, or the default one if the specified resource didn't
    override the requested filepath. Throws an exception if neither directory contained the
    filepath.
    If multiple arguments are provided, they're os.path.join()ed together.

    For example, if you want to load some json file:

        with open(get_path(json_dir_name, "myconfig.json")) as f:
            data = json.load(f)

    It will check the provided resource directory for the path, and then the default one if it
    doesn't exist, returning the first existing one.
    '''
    candidate_dirs = [src_dir, def_src_dir]
    for dir in candidate_dirs:
        x = join(dir, *args)
        if os.path.exists(x):
            return x
    raise IOError('Could not find %s in any of these dirs: %s'
                  % (join(*args), candidate_dirs))


def gen_header(filepath, contents):
    with open(get_path(json_dir_name, 'header.json')) as f:
        jsondata = json.load(f, object_pairs_hook=OrderedDict)
    innerHTML = gen_secondary_links(filepath, contents)

    # This is the orange text displayed beside the Legato logo
    # If possible, set it to the name of the context
    c = cat_for_link(os.path.basename(filepath))
    if c:
        displayed_category = c[0]
    else:
        displayed_category = "Legato Documentation"

    with open(get_path(template_dir_name, jsondata["template"])) as f:
        return f.read().format(innerHTML, displayed_category)


def gen_sidemenu(filepath, contents):
    """generate a hamburger menu"""

    with open(get_path(json_dir_name, "sidemenu.json")) as f:
        jsondata = json.load(f, object_pairs_hook=OrderedDict)

    innerHTML = ""
    links = jsondata["links"]

    for link in links:

        # Handle compatibility with Doxygen >= 1.6.11 which uses the actual name, not unix_style
        if link["check"] and not link["href"].startswith("http"):
            partfilename = re.sub(".html", ".part.html", link["href"])
            link_filepath = os.path.abspath(join(html_dir, partfilename))
            if not os.path.isfile(link_filepath):
                newfilename = re.sub("_([a-z])", lambda l: l.group(1).upper(),
                                     link["href"])  # file_name.html => fileName.html
                newpartfilename = re.sub(".html", ".part.html", newfilename)
                link_filepath = join(html_dir, newpartfilename)
                if not os.path.isfile(link_filepath):
                    raise Exception("Unable to find " + link["href"] + " or " + newfilename)
                link["href"] = newfilename

        innerHTML += format_sidemenu_link(jsondata["innerHTML"], link, filepath)

    # substitute in our links & title, and then render it just in case.
    return render_file(get_path(template_dir_name, jsondata["template"])).format(
        innerHTML, jsondata.get("title", ""))


def format_sidemenu_link(template, json_link, target_file, depth=0):
    """recursively render a json TOC node and its children as html links for use in the hamburger."""
    childrenHTML = ""
    if "children" in json_link:
        for c in json_link["children"]:
            childrenHTML += format_sidemenu_link(template,
                                                 c, target_file, depth + 1)
    element_class = ("navlink" if depth == 0 else "subnavlink")
    x = cat_for_link(target_file)
    if x and json_link["title"] == x[0]:
        element_class += " selected"
    return template.format(json_link["href"], 'class="' + element_class + '"', json_link["title"]) + childrenHTML


def gen_tocfile(filepath, contents):
    """returns the path to a toc json file based on filename"""
    filename = os.path.basename(filepath)
    x = cat_for_link(filename)
    if x is None:
        print "Not in TOC: " + filename
        return ""
    else:
        # the join(filter(None, x)) below ensures we only join non-empty strings
        # (otherwise we get 'Build_Apps_.json' when there's no category)
        return "tocs/" + ("_".join(filter(None, x))).replace(' ', '_') + ".json"


def gen_contextstring(filepath, contents):
    ctx, category = cat_for_link(os.path.basename(filepath)) or ('', '')
    return ctx




def gen_secondary_links(filepath, contents):
    """generate secondary nav links (e.g. Getting Started, Learn, Reference)"""
    innerHTML = '<a href="{0}"{1}>{2}</a>'
    output = ""
    x = cat_for_link(os.path.basename(filepath))
    if x:
        context_name, my_cat = x
        context = flat_contexts[context_name]
        for cat in context:
            output += innerHTML.format(context[cat][0], ' class="link-selected"' if my_cat == cat else "", cat)
    return output


# which tags get mapped to which functions
funcs = OrderedDict({
    "%%%TOCFILE%%%": gen_tocfile,
    "%%%HEADER%%%": gen_header,
    "%%%SIDEMENU%%%": gen_sidemenu,
    "%%%CONTEXT%%%": gen_contextstring
})


def render_file(filepath):
    with open(filepath, 'r') as f:
        return render(filepath, f.read())


def render(filepath, contents):
    """replace each tag in contents with the output of its corresponding function."""
    if args.verbose:
        print(filepath)
    for k in funcs.keys():
        if k in contents:
            contents = contents.replace(
                k, funcs[k](filepath, contents).encode())
    return contents


def meta_to_title(html):
    """extract the title of the document from the meta tag, where it was stashed by the extractHTML script.
    This is done because <title> isn't in part of the content div that we extract, so we put it in a <meta> tag
    to preserve it.
    """
    soup = BeautifulSoup(html, "lxml")
    titletag = soup.find("meta", attrs={"name": "title"})
    if titletag:
        titletag.extract()  # delete the meta tag
        title = titletag.attrs["content"]
        # because apparently soup.title is read only
        soup.html.head.title.string = title + " - Legato Docs"
    else:
        print "No title meta tag."
    return str(soup)


def get_tree_hrefs(tree):
    """get all urls in a tree as a flat list"""
    l = [tree['href']]
    if 'children' in tree:
        for c in tree['children']:
            l.extend(get_tree_hrefs(c))
    return l


# mangohDevelopers.html isn't actually in the toc, but we still want it to look as if it's part of Build Apps.
# so, the easy solution is to just pre-include it in this dict
cat_cache = {
'mangohDevelopers.html': ('Build Apps', '')
}


def cat_for_link(filename):
    """get the tuple (context, category) for a file
    e.g. ('Build Apps','Concepts')
    """
    # in case it's an absolute path
    if (html_dir_absolute + '/') in filename:
        filename = filename.replace(html_dir_absolute + '/', '')

    if filename in cat_cache:
        return cat_cache[filename]
    for context in contexts:
        if filename == context['href']:
            return context['label'], ""
    for context in flat_contexts:
        for cat in flat_contexts[context]:
            if filename in flat_contexts[context][cat] or \
                    filename.replace("_source.html", ".html") in flat_contexts[context][cat]:
                cat_cache[filename] = (context, cat)
                return context, cat


# copy tree recursively, overwriting files if they exist
def copytree(src, dest):
    if os.path.isdir(src):
        if not os.path.isdir(dest):
            os.makedirs(dest)
        for f in os.listdir(src):
            copytree(os.path.join(src, f), os.path.join(dest, f))
    else:
        dirname = os.path.dirname(dest)
        if not os.path.isdir(dirname):
            os.makedirs(dirname)
        shutil.copyfile(src, dest)


if __name__ == "__main__":
    print("src_dir = %s" % src_dir)

    if not os.path.exists(out_root_dir):
        os.makedirs(out_root_dir)

    if not os.path.exists(toc_dir):
        os.makedirs(toc_dir)

    with open(join(html_dir, "toc.json")) as f:
        data = json.load(f)


    if args.split_ctx:
        # split it at the context level,
        # so we get get a list of trees rooted at one of Build Apps, Build
        # Platform, etc.
        contexts = data['toc']['children']
    else:
        contexts = [data['toc']] # list of length 1, containing the root node

    # dictionary of contexts (key is the name). each context's value is another list of top-level categories,
    # and the values of those are a list of pages.
    flat_contexts = OrderedDict(
        (page['label'], OrderedDict(
            (cat['label'], get_tree_hrefs(cat))
            for cat in page['children']))
        for page in contexts)

    # For each context:
    for ctx in contexts:
        # Create an overview toc json. This appears on the top level page of each context, e.g. buildAppsMain.html
        # It is basically a list of categories with no depth, as loading the entire tree on every page would be slow and useless.
        with open(join(toc_dir, ctx['label'].replace(' ', '_') + ".json"), "w") as f:
            idx = {
                'id': 1,
                'children': [
                    {k: v for k, v in child.iteritems() if k != "children"}
                    for child in ctx['children']
                    ]
            }
            f.write("setupTree(%s)" % json.dumps(idx))
        # the actual toc files for each category
        for cat in ctx['children']:
            with open(join(toc_dir, (ctx['label'] + "_" + cat['label']).replace(' ', '_') + ".json"), "w") as f:
                f.write("setupTree(%s)" % json.dumps(cat))
    html_dir_absolute = join(os.getcwd(), html_dir)
    # walk through the input html directory
    for dir, subdirs, files in os.walk(html_dir_absolute):

        # rel_dir is the current directory being looked through, relative to html_dir
        # e.g. if dir = '[...]/$html_dir/misc_junk/', rel_dir='misc_junk/'
        # This is needed to preserve the original directory structure when evaluating write_dir,
        # so that the contents of that example dir will get written to '$out_root_dir/misc_junk'

        rel_dir = dir[len(html_dir_absolute):]

        write_dir = join(os.getcwd(), out_root_dir, rel_dir)
        if not os.path.exists(write_dir):
            os.makedirs(write_dir)

        for filename in files:
            filepath = join(dir, filename)
            if filepath.endswith(".html"):
                split = filename.split(".")
                # if the file is like foo.bar.html,
                # we want to insert the file into the bar.html template
                if len(split) > 2:
                    template = split[-2] + ".html"
                    resulting_filename = split[0] + ".html"
                    resulting_filepath = join(dir, resulting_filename)
                    with open(get_path(template_dir_name, template)) as f:
                        tp = f.read()
                    # render elements inside template
                    rendered_template = render(resulting_filepath, tp)
                    with open(filepath) as f:
                        contents = rendered_template.replace(
                            "{content}", f.read())  # insert content into rendered template
                    contents = meta_to_title(contents)
                    savefile = resulting_filename
                else:  # i.e. it doesn't use a template so just render any tags in it
                    contents = render_file(filepath, rel_dir)
                    savefile = filename
                with open(join(write_dir, savefile), 'w+') as f:
                    f.write(contents)

    print("Copying static files.")

    copytree(join(def_src_dir,static_dir_name), out_root_dir)
    copytree(get_path(static_dir_name), out_root_dir)

