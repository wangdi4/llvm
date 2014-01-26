#!/usr/bin/env python
import json, os

def render(lookup_path, data, kw):
    from mako.template import Template
    from mako.lookup import TemplateLookup
    lookup = TemplateLookup(directories=[lookup_path])
    return Template(data, lookup=lookup).render(**kw)

def main(argv=None):
    from os.path import isfile
    from sys import stdin

    if argv is None:
        import sys
        argv = sys.argv

    from optparse import OptionParser

    parser = OptionParser("usage: %prog [options] <template> <schema>")
    parser.add_option("-o", "--output", action="store", dest="output_fn",    help="output file name")

    opts, args = parser.parse_args(argv[1:])

    template_fn = args[0]
    if not isfile(template_fn):
        raise SystemExit("error: can't find template file %s" % template_fn)

    schema_fn = args[1]
    if not isfile(schema_fn):
        raise SystemExit("error: can't find json schema file %s" % schema_fn)

    lookup_path = os.path.dirname(os.path.realpath(template_fn))
        
    with open( template_fn ) as template_fo:
        template = template_fo.read()
        with open( schema_fn ) as schema_fo:
            schema_txt = schema_fo.read()
            schema = json.loads(schema_txt)
            output_txt = render(lookup_path, template, {"schema":schema})
            if None != opts.output_fn:
                with open(opts.output_fn, 'w') as output_fo:
                    output_fo.write(output_txt)
            else:
                print(output_txt)

if __name__ == "__main__":
    main()
