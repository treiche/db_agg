#!/usr/bin/python3

import argparse
import lxml
from xml.etree import ElementTree
import re
from xml.etree.ElementTree import parse
import sys
import os
from jinja2 import Environment, FileSystemLoader
from os.path import basename, dirname,exists

def literal(option):
    if option["type"] == "string":
        return '"' + option["default"] + '"'
    else:
        return option["default"]

def createTemplate(clidef,outputFile,template):
    jinjaEnv = Environment(
        loader=FileSystemLoader(dirname(__file__) + "/resources"),
        variable_start_string = "${",
        variable_end_string = "}"
    )
    jinjaEnv.filters["literal"] = literal
    jt = jinjaEnv.get_template(template)
    ti = jt.render(clidef.data)
    if not exists(dirname(outputFile)):
        os.makedirs(dirname(outputFile))
    output = open(outputFile,"wt")
    output.write(ti)
    output.close()


class CLDef:
    def __init__(self,src):
        self.data = {}
        self.data["optionGroups"] = []
        self.data["arguments"] = []
        self.parse(src)
        self.checkUniqueness()
        #print("data = "+str(self.data))

    def parse(self,src):
        #print("src="+src)
        root = ElementTree.parse(src).getroot()
        for el in root:
            if el.tag == "name":
                self.data["name"] = el.text
            if el.tag == "bean":
                self.data["bean"] = el.text
            if el.tag == "parser":
                self.data["parser"] = el.text
            if el.tag == "namespace":
                self.data["namespace"] = el.text
            if el.tag == "arguments":
                for arg in el:
                    self.data["arguments"].append(self.props(arg))
            if el.tag == "optionGroup":
                optionGroup = {}
                optionGroup["options"] = []
                for og in el:
                    if og.tag == "name":
                        optionGroup["name"] = og.text
                    if og.tag == "option":
                        optionGroup["options"].append(self.props(og))
                self.data["optionGroups"].append(optionGroup)

    def props(self,el):
        p = {}
        for child in el:
            p[child.tag] = child.text
        
        if "long" in p:
            asProp = self.xml_to_camelcase(p["long"]);
            p["property"] = asProp
            p["method"] = asProp[0].capitalize() + asProp[1:]
        if "name" in p:
            asProp = self.xml_to_camelcase(p["name"]);
            p["property"] = asProp
            p["method"] = asProp[0].capitalize() + asProp[1:]
        return p

    def xml_to_camelcase(self,name):
        def repl(mg):
            return mg.group(1).upper()
        return re.sub(r"-([a-z])",repl,name)

    def checkUniqueness(self):
        longOptions = {}
        shortOptions = {}
        for optionGroup in self.data["optionGroups"]:
            for option in optionGroup["options"]:
                longOption = option["long"]
                shortOption = option["short"]
                if longOption in longOptions:
                    raise Exception("option "+longOption + " exists."+str(longOptions[longOption]));
                longOptions[longOption] = option;
                if shortOption in shortOptions:
                    raise Exception("option "+shortOption + " exists."+str(shortOptions[shortOption]));
                shortOptions[shortOption] = option;


parser = argparse.ArgumentParser(
        description = 'Process queries on several databases.',
        epilog = 'Have fun !'
    )
parser.add_argument('clidef', help='the cli definition file')
parser.add_argument('-b','--bean-output-dir', help='the output directory for the bean class')
parser.add_argument('-p','--parser-output-dir', help='the output directory for the parser class')
parser.add_argument('-a','--asciidoc-output-dir', help='the output directory for the asciidoc file')
args = parser.parse_args()

clidef = CLDef(args.clidef)

if args.bean_output_dir:
    outputFile = args.bean_output_dir + "/" + clidef.data["bean"] + ".h"
    createTemplate(clidef, outputFile, "bean.h.jinja")
    outputFile = args.bean_output_dir + "/" + clidef.data["bean"] + ".cpp"
    createTemplate(clidef, outputFile, "bean.cpp.jinja")
if args.parser_output_dir:
    outputFile = args.parser_output_dir + "/" + clidef.data["parser"] + ".h"
    createTemplate(clidef, outputFile, "parser.h.jinja")
    outputFile = args.parser_output_dir + "/" + clidef.data["parser"] + ".cpp"
    createTemplate(clidef, outputFile, "parser.cpp.jinja")
if args.asciidoc_output_dir:
    outputFile = args.asciidoc_output_dir + "/cli.asciidoc"
    createTemplate(clidef, outputFile, "cli.asciidoc.jinja")
    
