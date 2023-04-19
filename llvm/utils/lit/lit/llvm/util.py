from lit.llvm.subst import ToolSubst

def add_default_options_to_tool(config, tool, options):
    optsub = ToolSubst(tool, unresolved='fatal')
    # Insert this first. Then, we'll first update the blank tool command; then,
    # the default substitution of tool will replace it to its full path.
    config.substitutions.insert(0, (optsub.regex,
        '{} {}'.format(tool,' '.join(options))))

def remove_default_options_from_tool(config, tool, options):
    optstr = ' '.join(options)
    for (x,y) in config.substitutions:
        if optstr in y and tool in x:
            # Remove the 1st one matched
            config.substitutions.remove((x,y))
            return
