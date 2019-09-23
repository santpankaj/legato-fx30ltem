import json
import sys
from os import system
from os.path import join
import re
import argparse
'''
Takes a toc json file created by extractDocStructure.py and flattens it, concatenating all the files it refers to into one BIIIG html file, which it writes to stdout.
html_dir/ is the directory where all your html files are.

ALSO:
 -adds anchors for each page
 -rewrites links pointing to other pages to point to anchors instead.
 -downgrades <h1>s without a class into <h2>s. This is because the <h1>s that we actually want to be top-level pdf TOC entries have class "title"
'''


def main():
    parser = argparse.ArgumentParser(
        description='''Takes a toc json file created by extractDocStructure.py and flattens it,
        concatinating all the files it refers to into one BIIIG html file, which it writes to stdout.''',
        formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument("toc_path", type=str,
                        help="Path to toc.json")
    parser.add_argument("source_path", type=str,
                        help="Location of HTML files")
    parser.add_argument('-p', '--paths', nargs="*", required=False,
                        help='''A list of specific sections to include
                                Example: -p "Build Apps > Concepts" "Build Apps > API Guides"''')
    args = parser.parse_args()

    j = json.load(open(args.toc_path))
    global links
    if args.paths:
        links = []

        for path in args.paths:
            split_path = path.split('>')
            node = j['toc']
            for node_str in split_path:
                node_str = node_str.strip()
                finds = [x for x in node['children'] if x['label'] == node_str]
                if not finds:
                    sys.stderr.write('Err: Node "%s" in query "%s" not found!\n' % (node_str, path))
                    sys.exit(1)
                node = finds[0]
            links.extend(uniq(flatten_node(node)))
    else:
        links = uniq(flatten_node(j['toc']))
    href_regex = re.compile(r'(<a[^>]* href=)"([^"]*)"([^>]*>)')
    h_regex = re.compile(r'<h(\d)[^>]*>(.+?)</h(\d)>', re.S)

    for link, depth in links:
        sys.stderr.write(str((link,depth))+'\n')
        with open(join(args.source_path, link)) as f:
            content = f.read()
        # write anchor at top of every page
        sys.stdout.write('<a name="%s"></a>' % link.replace(".part.html", ""))
        content = re.sub(href_regex, (lambda m: m.group(
            1) + convert_link(m.group(2),link) + m.group(3)), content)
        content = re.sub(h_regex, r'<span class="fake-h fake-h\1">\2</span>', content)
        title = re.search(r'<meta content="(.*?)" name="title"/>', content).group(1)
        sys.stdout.write('<h%s class="hide-hack">%s</h%s>' % (depth, title, depth))
        sys.stdout.write(content)


def convert_link(href, linked_from='[unknown]'):
    global links
    if '/' in href:
        return href  # external/relative link, dont touch it
    if '#' in href:
        return '#' + href.split('#')[1]
    else:
        if href.replace('.html','.part.html') not in links:
            sys.stderr.write('"%s" was linked to from "%s", but it doesn\'t seem to be present.\n' \
                % (href, linked_from.replace('.part.html','.html')))
        return '#' + href.replace(".html", "")


def flatten_node(node, hrefs=None, depth=0):
    hrefs = hrefs or []
    if 'href' in node and '#' not in node['href']:
        h = node['href'].replace(".html", ".part.html")
        hrefs.append((h, depth))
    if 'children' in node:
        for child in node['children']:
            hrefs.extend(flatten_node(child,depth=depth+1))
    return hrefs

# http://stackoverflow.com/a/480227/765210


def uniq(seq):
    seen = set()
    seen_add = seen.add
    return [x for x in seq if not (x in seen or seen_add(x))]

if __name__ == '__main__':
    main()
