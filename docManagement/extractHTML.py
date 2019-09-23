#!/usr/bin/env python2

import glob
import os
import argparse
import shutil
from tqdm import tqdm
from bs4 import BeautifulSoup
import re

def main():
    parser = argparse.ArgumentParser(description='Extract content from doxygenerated html to partial html files.')
    parser.add_argument("source_path", type=str,
                        help="Location of HTML files")
    parser.add_argument("dst_path", type=str,
                        help="Destination of converted partial HTML files")

    options = parser.parse_args()
    source_path=options.source_path
    dst_path=options.dst_path

    os.makedirs(dst_path)
    file_list=glob.glob(os.path.join(source_path,"*.html"))
    for f in tqdm(file_list):
        filename = os.path.basename(f)
        generate_html(source_path, filename, os.path.splitext(filename)[0] +  ".part.html",dst_path)


def generate_html(source_path, filename, dst_filename, dst_path):
    '''Loads doxygenerated html files, extracts the content, and the title if possible.
    It also undoes doxygen's conversion of -- to &mdash;
    '''
    with open(os.path.join(source_path,filename)) as f:
        html = "<pre>" + f.read() + "</pre>" # surround with pre tags so bs4 doesn't strip out whitespace

    html = html.decode("utf-8").replace(u'\u2013',"--").replace("&mdash;","--").replace("&ndash;","--")

    soup = BeautifulSoup(html,"lxml")

    # convert <div class="fragment"> to <pre class="fragment">
    for div in soup.find_all('div',{'class':'fragment'}):
        div.name = "pre"

    # convert <div class="line"></div> to <div class="line">&nbsp;</div>
    for div in soup.find_all('div',{'class':'line'}):
        if len(div.contents) == 0:
            div.string = u'\xa0'

    content=soup.find('div',{'class':'contents'})
    output = ""
    header = soup.find('div',{'class':'header'})

    if header:
        x = header.find('div',{'class':'title'})
        title =  "Legato Documentation" if x is None else str(x.string)
        if title == "Reference":
            title = "C Prototypes"
            x.string.replace_with(title)
        x.name = "h1"
        metatag = soup.new_tag('meta')
        metatag.attrs['name'] = 'title'
        metatag.attrs['content'] = title
        output = header.encode() + metatag.encode()

    # give copyright notice a separate tag, so it can be downplayed by css
    copyright = soup.find_all('p',text =re.compile(r'\s?Copyright \(C\) Sierra Wireless Inc\.( Use of this work is subject to license\.)?\s?'))
    for c in copyright:
        c.attrs['class'] = "copyright"

    output += content.encode(formatter='html')

    with open(os.path.join(dst_path,dst_filename),"w") as f:
        f.write(output)

if __name__ == '__main__':
    main()

