## @file mtv_xml_entities
#  This file contains most of the XML entities in use in MTV.
#  MTV system contains several XML data sources (such as mapfile.xml,
#  product SDF, user configuration file etc.).
#  For each XML data source, this file contains at least one class.
#  Each class in this file represents one level in the XML data source,
#  and its children.
#
#  For example, to represent an XML file containing three levels, we
#  need two classes: one class represents the first level of the XML file
#  and gives access to the second level, and the other class represents
#  the second level of the XML class, and its children (the third level).
#
#  This file defines a base class that all the classes inherit from.


from lxml import etree
import StringIO

XML_SCHEMA_BOOLEAN_TRUE  = ['true', 'True', 'TRUE']
XML_SCHEMA_BOOLEAN_FALSE = ['false', 'False', 'FALSE']

## Gets the text of the given node (the text between <> and </>)
#
#  @param nodes: The XML nodes
#
#  @return a text found between <> and </> of the given nodes
def GetElementText(nodes):
    if not nodes:
        return ""
    return nodes.firstChild.nodeValue.strip()

#---------------------------------------------#
# Base class for all XML-represented entities #
#---------------------------------------------#
## The base class for all the XML-represented entities.
#  This class provides the basic functionality of the other XML entities classes.
class XMLEntity:
    ## Each XML-representing class needs to be initialized with one of the two:
    #  a path to an XML file, or an element inside an existing XML document.
    #  in addition, an XML schema can be provided, to enable validation of the
    #  XML document.
    #
    #  @param xml_file string: the path to the XML file
    #  @param schema_file string: the path to the XML schema file
    #  @param doc_root XML element: the XML element from another parsed XML
    #         document, that should be the root of the current XML object.
    #         For example, this is useful for representing the second level
    #         of a three-level XML document. The XML class representing the
    #         first level can create an XML class and pass one of the elements
    #         of the second level as its doc_root. This causes the second class
    #         to represent the element passed to it, and all its children.
    def __init__(self, xml_file = None, schema_file = None, doc_root = None):
        if doc_root is not None:
            self._root = doc_root
        elif xml_file is not None:
            if schema_file is not None:
                self.load_validate(xml_file, schema_file)
            else:
                self.load(xml_file)
        else: # an empty XML container
            self._root = None

    ## Loads the XML document and creates the XML data structure.
    #  @param xml_file string: the path to the XML file
    def load(self, xml_file):
        xml_doc = etree.parse(xml_file)
        self._root = xml_doc.getroot()

    ## Loads the XML document, creates the XML data structure, and validates
    #  the XML document against the given XML schema.
    #
    #  @param xml_file string: the path to the XML file
    #  @param schema_file string: the path to the XML schema file
    #
    #  @raise ValueError: XML file was not validated successfully against the schema
    def load_validate(self, xml_file, schema_file):
        schema_doc = etree.parse(schema_file)
        xmlschema = etree.XMLSchema(schema_doc)
        xml_doc    = etree.parse(xml_file)
        if xmlschema.validate(xml_doc):
            self.load(xml_file)
        else:
            raise ValueError, xmlschema.error_log.last_error

    ## validates an already parsed XML document against the given XML schema.
    #
    #  @param schema_file string: the path to the XML schema file
    #
    #  @raise ValueError: XML file was not validated successfully against the schema
    def validate(self, schema_file):
        schema_doc = etree.parse(schema_file)
        xmlschema = etree.XMLSchema(schema_doc)
        xml_io = StringIO.StringIO(etree.tostring(self._root))
        xml_doc    = etree.parse(xml_io)
        if xmlschema.validate(xml_doc):
            return True
        else:
            raise ValueError, xmlschema.error_log.last_error

    ## writes the parsed XML document to a file. It can also validate the XML document
    #  against the given XML schema first.
    #
    #  @param xml_file string: the path to the new file that will contain the XML document
    #  @param schema_file string: the path to the XML schema file. If None is passed, the
    #         validation is skipped.
    def dump_to_file(self, xml_file, schema_file = None):
        if schema_file is not None:
            self.validate(schema_file)

        f = file(xml_file, 'w')
        f.write(etree.tostring(self._root, pretty_print=True))
        f.close()

    ## Convert the parsed XML document to a string. It can also validate the XML document
    #  against the given XML schema first.
    #
    #  @param schema_file string: the path to the XML schema file.
    #
    #  @return a string representing the XML file contents.
    def to_string(self, schema_file = None):
        if schema_file is not None:
            self.validate(schema_file)
        return etree.tostring(self._root, pretty_print=True)


    ## Return the root of the current parsed XML document
    #  @return root of the current XML document
    def get_root(self):
        return self._root


    ## Gets the attribute value of the current XML document
    #  @param name string: the attribute name
    #  @return the value of the attribute, or None if this attribute does not exist.
    def get_attribute_by_name(self, name):
        attr = self._root.attrib
        if not attr or not attr.has_key(name):
            return None
        return attr[name]

    ## Sets the attribute value of the current XML document
    #  @param name string: the name of the attribute
    #  @param value string: the new value for the attribute
    def set_attribute_by_name(self, name, value):
        self._root.set(name, value)

    ## Adds a child element to the root of the current XML document.
    #  This affects only the XML data structure, not the XML file.
    #
    #  @param name string: the name of the new element
    #  @return the new element in the XML document
    def add_child_by_name(self, name):
        element = etree.Element(name)
        self._root.append(element)
        return element

    ## Gets a list of elements, that are the children of the root of the current
    #  XML document, with the given name.
    #
    #  @param name string: the name of the required children elements
    #  @return a (possibly empty) list of children with the given name
    def get_children_by_name(self, name):
        children = []
        for el in self._root.iterchildren():
            if el.tag == name:
                children.append(el)
        return children

    ## Gets an element, that is the first child of the root of the current XML
    #  document, with the given name.
    #
    #  @param name string: the name of the required child element
    #  @return a single child with the given name. If more than one child exist
    #          with the given name, returns the first one. If no children with
    #          the given name are found, return None.
    def get_child_by_name(self, name):
        children = self.get_children_by_name(name)
        if (children):
            return children[0]
        return None

    ## Gets the text between <> and </> of the child with the given name of the root.
    #  @param name string: the name of the child
    #  @return the text of the child, or None when no children with the given
    #          name are found.
    def get_child_text_by_name(self, name):
        elem = self.get_child_by_name(name)
        if elem == None:
            return None
        else:
            return elem.text

    ## Sets the text between <> and </> of the child with the given name of the root.
    #  @param name string: the name of the child
    #  @param text string: the new text for the child
    #  @return the child element with the changed text
    def set_child_text_by_name(self, name, text):
        child = self.get_child_by_name(name)
        if child is None:
            child = self.add_child_by_name(name)
        child.text = text
        return child

    ## Gets a list of texts of all the children with the given name of the root.
    #  @param name string: the name of the children
    #  @return a (possibly empty) list of texts of the children with the given name.
    def get_children_texts_by_name(self, name):
        children = []
        for el in self._root.iterchildren():
            if el.tag == name:
                children.append(el.text)
        return children




#---------------------------------------------#
# A test result                               #
#---------------------------------------------#
## A class representing an XML result of task execution.
#  This class is special in the sense that each result can
#  contain sub-results of the same type. i.e. the results can
#  occur recursively. Therefore, the deeper levels of the XML
#  result file are represented by this class as well.
class XMLResult(XMLEntity):
    ## XMLResult constructor calls XMLEntity constructor.
    #  In addition, if the result is empty, it adds required elements to it.
    def __init__(self, xml_file = None, schema_file = None, doc_root = None):
        XMLEntity.__init__(self, xml_file, schema_file, doc_root)
        if self._root is None:
            self._root = etree.Element('Result', Name="")
            self._root.append(etree.Element('ExitStatus'))
            self._root.append(etree.Element('StartTime'))
            self._root.append(etree.Element('EndTime'))
            self._root.append(etree.Element('MachineName'))
            self._root.append(etree.Element('OutputDir'))
            self._root.append(etree.Element('SubResults'))
            self._root.append(etree.Element('ComponentType'))
            self._root.append(etree.Element('RepositoryPath'))
            self._root.append(etree.Element('Revision'))

    ## Get the name of the result entry (task, WP, test, etc.)
    def get_name(self):
        return self.get_attribute_by_name('Name')
    ## set the name of the result entry (task, WP, test, etc.)
    #  @param name string: the new name of the result entry
    def set_name(self, name):
        self.set_attribute_by_name('Name', name)

    ## Get the component type of the result entry (Task, Test, etc.)
    def get_component_type(self):
        return self.get_child_text_by_name('ComponentType')
    ## Set the component type of the result entry
    #  @param ctype string: The component type (Task, Test, etc.)
    def set_component_type(self, ctype):
        self.set_child_text_by_name('ComponentType', ctype)

    ## Get the exit status of the result entry (Pass, Fail, etc.)
    def get_exit_status(self):
        return self.get_child_text_by_name('ExitStatus')
    ## set the exit status of the result entry
    #  @param status string: the new exit status (Pass, Fail, Abort, etc.)
    def set_exit_status(self, status):
        self.set_child_text_by_name('ExitStatus', status)

    ## Get the execution start time of the result entry (YYYY-MM-DD HH-MM-SS)
    def get_start_time(self):
        return self.get_child_text_by_name('StartTime')
    ## set the execution start time of the result entry
    #  @param stime string: string representation of the start time (YYYY-MM-DD HH-MM-SS)
    def set_start_time(self, stime):
        self.set_child_text_by_name('StartTime', stime)

    ## get the execution end time of the result entry (YYYY-MM-DD HH-MM-SS)
    def get_end_time(self):
        return self.get_child_text_by_name('EndTime')
    ## set the execution end time of the result entry
    #  @param etime string: string representation of the end time (YYYY-MM-DD HH-MM-SS)
    def set_end_time(self, etime):
        self.set_child_text_by_name('EndTime', etime)

    ## get the name of the machine executed the job
    def get_machine_name(self):
        return self.get_child_text_by_name('MachineName')
    ## set the name of the machine executed the job
    #  @param name string: the name of the machine
    def set_machine_name(self, name):
        self.set_child_text_by_name('MachineName', name)

    ## get the output directory path of the result entry (not of the result file)
    def get_output_dir(self):
        return self.get_child_text_by_name('OutputDir')
    ## set the output directory path of the result entry (not of the result file)
    #  @param odir string: the path of the directory containing the job of the result entry
    def set_output_dir(self, odir):
        self.set_child_text_by_name('OutputDir', odir)

    ## get the repository path used for the job of this result entry
    def get_repository_path(self):
        return self.get_child_text_by_name('RepositoryPath')
    ## set the repository path used for the job of this result entry
    #  @param path string: the repository path
    def set_repository_path(self, path):
        self.set_child_text_by_name('RepositoryPath', path)

    ## get the repository revision used for the job of this result entry
    def get_revision(self):
        return self.get_child_text_by_name('Revision')
    ## set the repository revision used for the job of this result entry
    #  @param rev string: the repository revision
    def set_revision(self, rev):
        self.set_child_text_by_name('Revision', rev)

    ## get a list of sub-results for this result entry.
    #  each result entry can contain many other results.
    #
    #  @return XMLResult object representing the sub-results, or None if there are
    #          no sub-results
    def get_sub_results(self):
        sub_results_list = []
        sub_results = self.get_child_by_name("SubResults")
        for result in sub_results:
            sub_results_list.append(XMLResult(doc_root = result))
        return sub_results_list
    ## Add a sub-result to this result entry
    #  @param result XMLResult: the XMLResult object for the new sub-result
    def add_sub_result(self, result):
        sub_result = self.get_child_by_name('SubResults')
        sub_result.append(result.get_root())

    ## Checks if the current result entry contains any sub-results.
    #
    #  @return True if the result entry has sub-results, False otherwise
    def has_sub_results(self):
        sub_results = self.get_sub_results()
        return len(sub_results) > 0

    ## convert the result document to a string
    #
    #  @return string representing the XML results object.
    def to_string(self):
        return etree.tostring(self._root, pretty_print=True)

    ## get the reason for the exit status of the current result entry,
    #  or an empty string if no reason exist.
    def get_exit_reason(self):
        #  ExitReason is optional and may not exist
        ret = self.get_child_text_by_name('ExitReason')
        if ret is None:
            return ""
        return ret
    ## set the reason for the exit status of the current result entry.
    #  @param reason string: the new reason description.
    def set_exit_reason(self, reason):
        if reason is None:
            return None
        #  ExitReason is optional and may not exist
        if self.get_child_text_by_name('ExitReason') is None:
            self._root.insert(1, etree.Element('ExitReason'))
        self.set_child_text_by_name('ExitReason', reason)


#-----------------------------------#
# A component in a productSDF file  #
#-----------------------------------#
## This class represents a component in the product SDF document.
#  This is the second level of the product SDF document.
class XMLProductSDFComponent(XMLEntity):
    ## calls the XMLEntity constructor
    def __init__(self, xml_file = None, schema_file = None, doc_root = None):
        XMLEntity.__init__(self, xml_file, schema_file, doc_root)

    ## get the component name
    def get_name(self):
        return self.get_attribute_by_name("Name")

    ## get the component relative checkout path
    def get_relative_checkout_path(self):
        return self.get_child_text_by_name("RelativeCheckoutPath")

    ## get the component repository path. This exists only if differs than
    #  the repository defined in the map file. Therefore, None can be returned.
    def get_repository_path(self):
        return self.get_child_text_by_name("RepositoryPath")

    ## get the component relative path in the repository. This exists only if
    #  differs than the relative path defined in the map file. Therefore, None
    #  can be returned.
    def get_relative_path_in_repository(self):
        return self.get_child_text_by_name("RelativePathInRepository")

    ## get the component repository tag. This is not mandatory and therefore
    #  None can be returned.
    def get_tag(self):
        return self.get_child_text_by_name("Tag")

    ## print the SDF component to standard output
    def xprint(self):
        print 'Name:', self.get_name()
        print 'RelativeCheckoutPath:', self.get_relative_checkout_path()
        print 'RepositoryPath:', self.get_repository_path()
        print 'RelativePathInRepository:', self.get_relative_path_in_repository()
        print 'Tag:', self.get_tag()

#---------------------------------------------#
# A ProductSDF file                           #
#---------------------------------------------#
## This class represents the product SDF document. This class provides access
#  to the product SDF components by returning XMLProductSDFComponent objects.
class XMLProductSDF(XMLEntity):
    ## calls the XMLEntity constructor
    def __init__(self, xml_file = None, schema_file = None, doc_root = None):
        XMLEntity.__init__(self, xml_file, schema_file, doc_root)

    ## get the name of the product that the SDF document belongs to.
    def get_name(self):
        return self.get_attribute_by_name("Name")

    ## get the product mailing list
    def get_mailing_list(self):
        return self.get_child_text_by_name("MailingList")

    ## get the product owner name
    def get_owner(self):
        return self.get_child_text_by_name("Owner")

    ## get a list of the product SDF components (XMLProductSDFComponent objects)
    def get_components(self):
        comp_list = []
        components = self.get_children_by_name("Component")
        if (components):
            for comp in components:
                comp_list.append(XMLProductSDFComponent(doc_root = comp))
        return comp_list

    ## get a single SDF component with the given name, or None if one does not exist
    #  @param name string: the component name
    def get_component_by_name(self, name):
        components = self.get_components()
        for comp in components:
            comp_name = comp.get_attribute_by_name("Name")
            if comp_name == name:
                return comp
        return None

    ## get a boolean result indicating if the product has components
    def has_components(self):
        return len(self.get_components()) != 0

    ## print the entire product SDF document (including components) to standard output.
    def xprint(self):
        print 'Name:', self.get_name()
        print 'Mailing list:', self.get_mailing_list()
        print 'Owner:', self.get_owner()
        if self.has_components():
            print 'Components: '
            for comp in self.get_components():
                print '----------------------------------------------------------------'
                comp.xprint()


#---------------------------------------------#
# A command line in a TestType                #
#---------------------------------------------#
## this class represents a command line in the test-type definition file
class XMLCommandLine(XMLEntity):
    ## calls the XMLEntity constructor
    def __init__(self, xml_file = None, schema_file = None, doc_root = None):
        XMLEntity.__init__(self, xml_file, schema_file, doc_root)

    ## get the Launch (main) command  line.
    def get_launch(self):
        return self.get_child_text_by_name('Launch')

    ## get the OS type for which the commands are supported
    def get_os_type(self):
        return self.get_child_text_by_name('OSType')

    ## return True if the pre-command exist, False otherwise.
    #  When exists, pre-command is executed before the Launch command.
    def has_pre(self):
        pre = self.get_pre()
        return pre != None
    ## get the pre-command.
    #  When exists, pre-command is executed before the Launch command.
    def get_pre(self):
        return self.get_child_text_by_name('Pre')

    ## return True id the post-command exist, False otherwise.
    #  When exists, post-command is executed after the Launch command.
    def has_post(self):
        post = self.get_post()
        return post != None
    ## get the post-command.
    #  When exists, post-command is executed after the Launch command.
    def get_post(self):
        return self.get_child_text_by_name('Post')


#---------------------------------------------#
# A Test's file needs to be attached          #
#---------------------------------------------#
## this class represents a file that should be attached to
#  the email after the test is executed.
class XMLTestTypeFile(XMLEntity):
    ## calls the XMLEntity constructor
    def __init__(self, xml_file = None, schema_file = None, doc_root = None):
        XMLEntity.__init__(self, xml_file, schema_file, doc_root)

    ## get the path of the file
    def get_path(self):
        return self.get_child_text_by_name('Path')

    ## return True if the file should be attached when the test passes, False otherwise
    def file_needed_in_pass(self):
        if self.get_attribute_by_name('Pass') in XML_SCHEMA_BOOLEAN_TRUE:
            return True
        return False

    ## return True if the file should be attached when the test fails, False otherwise.
    def file_needed_in_fail(self):
        if self.get_attribute_by_name('Fail') in XML_SCHEMA_BOOLEAN_TRUE:
            return True
        return False


#---------------------------------------------#
# A TestType                                  #
#---------------------------------------------#
## this class represents a test-type definition XML file.
#  the test-type XML definition file has more than two levels
#  and therefore this class creates other objects that represent
#  the deeper elements of the test-type XML definition file.
class XMLTestType(XMLEntity):
    ## calls the XMLEntity constructor
    def __init__(self, xml_file = None, schema_file = None, doc_root = None):
        XMLEntity.__init__(self, xml_file, schema_file, doc_root)

    ## get the name of the test-type component
    def get_name(self):
        return self.get_attribute_by_name('Name')

    ## get the description of the test-type
    def get_description(self):
        return self.get_child_text_by_name('Description')

    ## get a list of commands defined for this test-type. Each command
    #  is an object of type XMLCommandLine.
    def get_cmd_list(self):
        cmd_list = []
        cmds = self.get_children_by_name('Cmd')

        for cmd in cmds:
            cmd_list.append(XMLCommandLine(doc_root = cmd))

        return cmd_list

    ## get one of the commands defined for this test-type, that matches the given OS.
    #  @param os_type string: the required OS type of the command (Windows, etc.)
    def get_cmd_by_os_type(self, os_type):
        cmds = self.get_cmd_list()

        for cmd in cmds:
            if cmd.get_os_type() == os_type:
                return cmd

        return None

    ## get a list of files-to-attach. Each file-to-attach is an object of type
    #  XMLTestTypeFile.
    def get_test_type_files(self):
        files_list = []
        filesToAttach = self.get_children_by_name('OutputFile')
        for f in filesToAttach:
            files_list.append(XMLTestTypeFile(doc_root = f))
        return files_list


#---------------------------------------------#
# SupportedPlatform                           #
#---------------------------------------------#
## This class represents a Supported Platform definition.
#  A supported platform is defined by its OS type and machine architecture.
#  OS type can be Windows, Linux and Mac OSx.
#  Machine architecture can be IA32 or IA64
class XMLSupportedPlatform(XMLEntity):
    ## calls the XMLEntity constructor
    def __init__(self, xml_file = None, schema_file = None, doc_root = None):
        XMLEntity.__init__(self, xml_file, schema_file, doc_root)

    ## get the supported platform's OS type
    def get_os(self):
        return self.get_child_text_by_name('OS')

    ## get the supported platform's architecture
    def get_arch(self):
        return self.get_child_text_by_name('Architecture')

    ## print the supported platform
    def xprint(self):
        print 'OS:', self.get_os()
        print 'Architecture:', self.get_arch()

#---------------------------------------------#
# A component in a MapFile                    #
#---------------------------------------------#
## This class represents a component from the map file
class XMLMapFileComponent(XMLEntity):
    ## calls the XMLEntity constructor
    def __init__(self, xml_file = None, schema_file = None, doc_root = None):
        XMLEntity.__init__(self, xml_file, schema_file, doc_root)

    ## get the repository name of the component.
    #  the repository name is defined in the map file, and it represents a
    #  base URL to the repository that contains the component.
    def get_repository_name(self):
        return self.get_child_text_by_name('RepositoryName')

    ## get the relative path in the repository for the component.
    #  the relative path in the repository represents the component's location
    #  in the repository. The absolute path for the component can be constructed
    #  by concatenating the repository base URL with the repository relative path.
    #
    #  There is an option to override this value (use override_repository_path).
    def get_relative_path_in_repository(self):
        if hasattr(self, "_RepoPath"):
            return self._RepoPath
        return self.get_child_text_by_name('RelativePathInRepository')

    ## get the name of the component's owner.
    def get_owner(self):
        return self.get_child_text_by_name('Owner')

    ## get the component type (can be Package, Test, TestType, Profile, Workplan, etc.)
    def get_component_type(self):
        return self.get_child_text_by_name('ComponentType')

    ## get the mailing list for the component.
    #  the mailing list contains email addresses of the people interested in the component.
    #  This mailing list can be used to send announcements regarding that component.
    def get_mailing_list(self):
        return self.get_child_text_by_name('MailingList')

    ## get the component's name. This name is the component's identifier in MTV.
    def get_name(self):
        return self.get_attribute_by_name('Name')

    ## get a list of supported platforms for the component.
    #  each supported platform is of type XMLSupportedPlatform.
    def get_supported_platforms(self):
        platforms_list = []
        platforms = self.get_children_by_name("SupportedPlatform")
        if (platforms):
            for plat in platforms:
                platforms_list.append(XMLSupportedPlatform(doc_root = plat))
        return platforms_list

    ## takes an alternative path in the repository and use it instead of the
    #  component's relative path in the repository.
    #
    #  @param path string: the new relative path in the repository
    def override_repository_path(self, path):
        #Boaz: Need to check with Doron
        if not path is None:
            self._RepoPath = path

    ## print the map component
    def xprint(self):
        print 'Name:', self.get_name()
        print 'RepositoryName:', self.get_repository_name()
        print 'RelativePathInRepository:', self.get_relative_path_in_repository()
        print 'Owner:', self.get_owner()
        print 'ComponentType:', self.get_component_type()
        print 'MailingList:', self.get_mailing_list()

        platforms = self.get_supported_platforms()
        for platform in platforms:
            print 'Supported Platform:'
            print '------------------------------------------------------------'
            print platform.xprint()


#---------------------------------------------#
# A repository in MapFile                     #
#---------------------------------------------#
## This class represents a repository defined in the map file
class XMLMapFileRepository(XMLEntity):
    ## calls the XMLEntity constructor
    def __init__(self, xml_file = None, schema_file = None, doc_root = None):
        XMLEntity.__init__(self, xml_file, schema_file, doc_root)

    ## get the name of the repository. This name is the repository's identifier
    #  in MTV.
    def get_name(self):
        return self.get_attribute_by_name("Name")

    ## get the URL to the repository
    def get_repository_path(self):
        return self.get_child_text_by_name("RepositoryPath")

    ## print the repository definition
    def xprint(self):
        print 'Name:', self.get_name()
        print 'RepositoryPath:', self.get_repository_path()


#---------------------------------------------#
# A MapFile                                   #
#---------------------------------------------#
## This class represents the map file of MTV system.
#  This map file contains a definition of all the SVN repositories being used
#  in MTV.
#  In addition, the map file contains a definition of all the components being
#  used in MTV. These components include the different product's components,
#  all the test-types, tests, workplans and profiles being used in MTV, etc.
class XMLMapFile(XMLEntity):
    ## calls the XMLEntity constructor
    def __init__(self, xml_file = None, schema_file = None, doc_root = None):
        XMLEntity.__init__(self, xml_file, schema_file, doc_root)

    ## get a list of all the components defined in the map file.
    #  each component is of type XMLMapFileComponent.
    def get_components(self):
        comp_list = []
        components = self.get_children_by_name("Component")
        if (components):
            for comp in components:
                comp_list.append(XMLMapFileComponent(doc_root = comp))
        return comp_list

    ## get a single component from the map file, by the component's name.
    #  the returned component is of type XMLMapFileComponent, or None if
    #  a component with the given name does not exist in the map file.
    #
    #  @param name string: the component name
    def get_component_by_name(self, name):
        components = self.get_components()
        for component in components:
            if component.get_name() == name:
                return component
        return None

    ## get a list of all the repositories defined in the map file.
    #  each repository is of type XMLMapFileRepository.
    def get_repositories(self):
        repo_list = []
        repositories = self.get_children_by_name("Repository")
        if (repositories):
            for repo in repositories:
                repo_list.append(XMLMapFileRepository(doc_root = repo))
        return repo_list

    ## get a single repository from the map file, by the repository's name.
    #  the returned repository is of type XMLMapFileRepository, or None, if
    #  a repository with the given name does not exist in the map file.
    #
    #  @param name string: the repository name
    def get_repository_by_name(self, name):
        repositories = self.get_repositories()
        for repo in repositories:
            if repo.get_name() == name:
                return repo
        return None

    ## get the repository being used in the given component's definition, by
    #  the component's name. The returned repository is of type XMLMapFileRepository,
    #  or None, if the given component does not exist in the map file, or if the given
    #  component's repository does not exist in the map file (this is a bug in the map
    #  file, and in this case, the map file must be modified to correct the problem).
    #
    #  @param name string: the component name, who's repository is returned
    def get_repository_by_component_name(self, name):
        component = self.get_component_by_name(name)
        if component is not None:
            repository = self.get_repository_by_name(component.get_repository_name())
            if repository is not None:
                return repository
        return None

    ## print the map file
    def xprint(self):
        repo_list = self.get_repositories()
        for repo in repo_list:
            print 'Repository:'
            print '------------------------------------------------------------'
            repo.xprint()

        comp_list = self.get_components()
        for comp in comp_list:
            print 'Component:'
            print '------------------------------------------------------------'
            comp.xprint()

#---------------------------------------------#
# A TestsFile                                   #
#---------------------------------------------#
## This class represents the tests file being used in MTV
class XMLTestsFile(XMLEntity):
    ## calls the XMLEntity constructor
    def __init__(self, xml_file = None, schema_file = None, doc_root = None):
        XMLEntity.__init__(self, xml_file, schema_file, doc_root)

    ## get a list of all the tests defined in the tests file.
    # each of the tests is of type XMLTest.
    def get_tests(self):
        test_list = []
        tests = self.get_children_by_name("Test")
        if (tests):
            for test in tests:
                test_list.append(XMLTest(doc_root = test))
        return test_list

    ## get a list of all the test-types found in the tests file.
    #  Each test in the map file contains the name of the test-type it
    #  implements. This function returns a list of the names of all the
    #  test-types that the tests in the tests file implement.
    def get_test_type_names(self):
        tests = self.get_tests()
        test_types = []
        for test in tests:
            test_type_name = test.get_test_type_name()
            if test_type_name not in test_types:
                test_types.append(test_type_name)
        return test_types

    ## get a single test by the given test name.
    #  the returned test is of type XMLTest, or None if a test with the given
    #  name does not exist in the tests file.
    #
    #  @param name string: the test's name
    def get_test_by_name(self, name):
        tests = self.get_tests()
        for test in tests:
            if test.get_name() == name:
                return test
        return None

    ## get a list of all the tests from the tests file, that implement a
    #  given test-type.
    #  the returned tests is of type XMLTest.
    #
    #  @param type_name string: the test-type name
    def get_tests_by_test_type(self, type_name):
        tests = []
        all_tests = self.get_tests()
        for test in all_tests:
            if test.get_test_type_name() == type_name:
                tests.append(test)
        return tests

#---------------------------------------------#
# A test in a Workplan                        #
#---------------------------------------------#
## This class represents a test defined in a workplan file.
class XMLWorkplanTest(XMLEntity):
    ## calls the XMLEntity constructor
    def __init__(self, xml_file = None, schema_file = None, doc_root = None):
        XMLEntity.__init__(self, xml_file, schema_file, doc_root)

    ## get the test's name
    def get_name(self):
        return self.get_attribute_by_name('Name')

    ## get a list of test names that must be executed before the current test.
    def get_dependencies(self):
        dependencies_list = []

        dependencies = self.get_children_by_name('Depends')

        for dependency in dependencies:
            if dependency.attrib.has_key('Name'):
                dependencies_list.append(dependency.attrib['Name'])

        return dependencies_list




#---------------------------------------------#
# A Workplan                                  #
#---------------------------------------------#
## This class represents a workplan file.
#  Workplan file contains a list of tests, with dependencies definition.
#  The dependency list of a test in the workplan file can contain list of
#  test names. Each of the tests in a dependency list must be executed
#  before the test that contain the dependency list.
class XMLWorkPlan(XMLEntity):
    ## calls the XMLEntity constructor
    def __init__(self, xml_file = None, schema_file = None, doc_root = None):
        XMLEntity.__init__(self, xml_file, schema_file, doc_root)

    ## get the name of the workplan. This name uniquely identifies the workplan
    #  in MTV.
    def get_name(self):
        return self.get_attribute_by_name('Name')

    ## get the description of the workplan. This field is optional and can be missing
    #  from the workplan file. When the description is missing, None is returned.
    def get_description(self):
        return self.get_child_text_by_name('Description')

    ## get a list of all the tests that need to be executed as part of the workplan.
    def get_tests(self):
        tests_list = []
        tests = self.get_children_by_name("Test")

        for test in tests:
            tests_list.append(XMLWorkplanTest(doc_root = test))
        return tests_list


#---------------------------------------------#
# A workplan in a Profile                     #
#---------------------------------------------#
## This class represents a workplan definition in a profile file.
class XMLProfileWorkplan(XMLEntity):
    ## calls the XMLEntity constructor
    def __init__(self, xml_file = None, schema_file = None, doc_root = None):
        XMLEntity.__init__(self, xml_file, schema_file, doc_root)

    ## get the name of the workplan
    def get_name(self):
        return self.get_attribute_by_name('Name')

    ## get a list of workplan names that must be executed before the current workplan.
    def get_dependencies(self):
        dependencies_list = []

        dependencies = self.get_children_by_name('Depends')

        for dependency in dependencies:
            if dependency.attrib.has_key('Name'):
                dependencies_list.append(dependency.attrib['Name'])

        return dependencies_list

#---------------------------------------------#
# A Profile                                   #
#---------------------------------------------#
## This class represents a profile file.
#  Each profile file contains a list of workplans that need to be executed
#  as part of the profile execution.
#  In addition, a dependency list can be defined for each of these workplans.
#  The workplan's dependency list contains the names of the workplans that
#  must be executed before the workplan that contains the dependency list.
class XMLProfile(XMLEntity):
    ## calls the XMLEntity constructor
    def __init__(self, xml_file = None, schema_file = None, doc_root = None):
        XMLEntity.__init__(self, xml_file, schema_file, doc_root)

    ## get the profile name. This name is the unique identifier of the profile
    #  in MTV
    def get_name(self):
        return self.get_attribute_by_name('Name')

    ## get the profile's description.
    #  the description is an optional field and can be missing from the profile's
    #  definition. If the description is missing, None is returned.
    def get_description(self):
        return self.get_child_text_by_name('Description')

    ## get a list of all the workplans that need to be executed as part of the
    #  profile. Each workplan may contain a list of dependencies on other workplans.
    #  The dependency list contains names of other workplans that must be executed
    #  before the current workplan.
    def get_workplans(self):
        wokrplans_list = []
        workplans = self.get_children_by_name("Workplan")

        for workplan in workplans:
            wokrplans_list.append(XMLProfileWorkplan(doc_root = workplan))
        return wokrplans_list


#---------------------------------------------#
# A Machine in a PoolConfig                   #
#---------------------------------------------#
## This class represents a machine in the MTV pool configuration.
#  Each machine is characterized by its hardware configuration, OS,
#  location and network domain.
#  This charecterization enables querying the pool and getting a list
#  of machines that match an execution profile.
class XMLMachine(XMLEntity):
    ## calls the XMLEntity constructor.
    #  If an empty object is created, the root of the XML object is created as well.
    def __init__(self, xml_file = None, schema_file = None, doc_root = None):
        XMLEntity.__init__(self, xml_file, schema_file, doc_root)
        if self._root is None:
            self._root = etree.Element('Machine')

    ## get the OS type. This can be Windows, Linux, etc.
    def get_os(self):
        return self.get_child_text_by_name('OS')
    ## set the machine's OS type.
    #  @param os string: the OS type
    def set_os(self,os):
        self.set_child_text_by_name('OS',os)

    ## get the machine's CPU type
    def get_cpu(self):
        return self.get_child_text_by_name('CPU')
    ## set the machine's CPU type
    #  @param cpu string: the CPU type
    def set_cpu(self,cpu):
        self.set_child_text_by_name('CPU',cpu)

    ## get the machine's architecture (either IA32 or IA64)
    def get_arch(self):
        return self.get_child_text_by_name('Architecture')
    ## set the machine's architecture
    #  @param arch string: the machine's architecture
    def set_arch(self,arch):
        self.set_child_text_by_name('Architecture',arch)

    ## get the machine's chipset
    def get_chipset(self):
        return self.get_child_text_by_name('Chipset')
    ## set the machine's chipset
    #  @param chipset string: the chipset
    def set_chipset(self,chipset):
        self.set_child_text_by_name('Chipset',chipset)

    ## get the machine's hostname
    def get_hostname(self):
        return self.get_attribute_by_name('HostName')
    ## set the machine's hostname
    #  @param hostname string: the hostname
    def set_hostname(self, hostname):
        self.set_attribute_by_name('HostName',hostname)

    ## get the domain that the machine is found in
    def get_domain(self):
        return self.get_attribute_by_name('Domain')
    ## set the machine's domain
    #  @param domain string: the domain
    def set_domain(self, domain):
        self.set_attribute_by_name('Domain',domain)

    ## get the machine's location
    def get_location(self):
        return self.get_attribute_by_name('Location')
    ## set the machine's location
    #  @param location string: machine's location
    def set_location(self, location):
        self.set_attribute_by_name('Location',location)

    ## get the display adapter installed on the machine
    def get_display_adapter(self):
        return self.get_child_text_by_name('DisplayAdapter')
    ## set the display adapter installed on the machine
    #  @param adapter string: the display adapter
    def set_display_adapter(self, adapter):
        self.set_child_text_by_name('DisplayAdapter',adapter)

    ## get the memory (amount) installed on the moachine
    def get_memory(self):
        return self.get_child_text_by_name('Memory')
    ## set the memory (amount) installed on the machine
    #  @param memory string: the memory
    def set_memory(self,memory):
        self.set_child_text_by_name('Memory',memory)

##---------------------------------------------#
## A PoolConfig                                #
##---------------------------------------------#
#class XMLPoolConfig(XMLEntity):
#    def __init__(self, xml_file = None, schema_file = None, doc_root = None):
#        XMLEntity.__init__(self, xml_file, schema_file, doc_root)
#
#
#    def get_machines(self):
#        machines_list = []
#        machines = self.get_children_by_name("Machine")
#
#        for machine in machines:
#            machines_list.append(XMLMachine(doc_root = machine))
#        return machines_list


#---------------------------------------------#
# A Job in a Task                             #
#---------------------------------------------#
## This class represents a job defined in a task.
class XMLJob(XMLEntity):
    ## calls the XMLEntity constructor.
    #  In addition, if an empty object is created, some of its elements
    #  are created as well, to provide the basic set of required fields.
    def __init__(self, xml_file = None, schema_file = None, doc_root = None):
        XMLEntity.__init__(self, xml_file, schema_file, doc_root)
        if self._root is None:
            self._root = etree.Element('Job')
            self._root.append(etree.Element('Name'))
            self._root.append(etree.Element('Revision'))

    ## get the job name
    def get_name(self):
        return self.get_child_text_by_name('Name')

    ## set the job name
    #  @param name string: the job name
    def set_name(self, name):
        self.set_child_text_by_name('Name', name)

    ## get the repository path defined for the job.
    #  This enables a task creator to define alternative location in
    #  the repository for the components in the task.
    def get_repository_path(self):
        return self.get_child_text_by_name('RepositoryPath')

    ## get the revision defined for the job.
    #  This enables the task creator to define alternative revision
    #  for the components in the task, and hence to execute different
    #  versions of the same component.
    def get_revision(self):
        return self.get_child_text_by_name('Revision')

    ## set the revision defined for the job.
    #  @param revision string: the revision
    def set_revision(self, revision):
        return self.set_child_text_by_name('Revision', revision)


#---------------------------------------------#
# A Task                                      #
#---------------------------------------------#
## This class represents a task file in MTV.
#  task file includes list of jobs and other information.
#  Each job in a task can be one of the types: Test, Workplan, Profile.
#  Other information (except of the jobs) is:
#  - list of machines that define a query on the machines pool that determines
#    the actual list of machines that will execute the task.
class XMLTask(XMLEntity):
    ## calls the XMLEntity constructor
    #  If an empty object is created, its root element is created as well.
    def __init__(self, xml_file = None, schema_file = None, doc_root = None):
        XMLEntity.__init__(self, xml_file, schema_file, doc_root)
        if self._root is None:
            self._root = etree.Element('Task', Name="")

    ## get the task name
    def get_name(self):
        return self.get_attribute_by_name('Name')
    ## set the task name
    #  @param name string: the task name
    def set_name(self, name):
        self.set_attribute_by_name('Name', name)

    ## get a list of jobs defined in the task.
    #  each job is of type XMLJob.
    def get_jobs(self):
        jobs_list = []
        jobs = self.get_children_by_name('Job')

        for job in jobs:
            jobs_list.append(XMLJob(doc_root = job))
        return jobs_list

    ## add a job to the task
    #  @param name string: the job name
    #  @param revision string: the revision for this job
    def add_job(self, name, revision):
        job = XMLJob()
        job.set_name(name)
        job.set_revision(revision)
        self._root.append(job.get_root())

    ## get a list of machines for this task.
    #  each machine is of type XMLMachine.
    #  The machines can define an existing machine in the pool, or a template
    #  for a machine with desired properties (then a query on the pool is performed)
    def get_machines(self):
        machines_list = []
        machines = self.get_children_by_name('Machine')

        for machine in machines:
            machines_list.append(XMLMachine(doc_root = machine))
        return machines_list

    ## get an indicator that determines if the task is defined with machines or not.
    def has_machines(self):
        return len(self.get_machines()) > 0

    ## get the mailing list defined in the task.
    #  The mailing list defines the list of email addresses that will receive the
    #  task execution's result via mail (if the execution includes mail notification).
    def get_mailing_list(self):
        return self.get_child_text_by_name('MailingList')

#    def send_notification_mail(self):
#        return self.get_child_text_by_name('SendNotificationMail')
#
#    def store_results_in_db(self):
#        return self.get_child_text_by_name('StoreDBResults')
#
#    def enable_test_type_db(self):
#        return self.get_child_text_by_name('EnableTestTypeDB')
#

    ## adds additional list of email addresses to the defined mailing list.
    #  @param additional_mailing_list string: a semicolon separated list of
    #         email addresses.
    def add_mailing_list(self, additional_mailing_list):
        current_mailing_list = self.get_mailing_list()
        if current_mailing_list is None:
            current_mailing_list = ''
        mailing_list = current_mailing_list + " ; " + additional_mailing_list
        self.set_child_text_by_name('MailingList', mailing_list)

    ## print the task
    def show(self):
        print 'I am a Task file. My name is', self.get_name()
        print 'I have ', len(self.get_jobs()), 'jobs'

#---------------------------------------------#
# A Test in the system                        #
#---------------------------------------------#
## This class represents a test defined in the tests file of MTV.
class XMLTest(XMLEntity):
    ## calls the XMLEntity constructor
    def __init__(self, xml_file = None, schema_file = None, doc_root = None):
        XMLEntity.__init__(self, xml_file, schema_file, doc_root)

    ## get the test name
    def get_name(self):
        return self.get_attribute_by_name('Name')

    ## get the name of the test-type implemented by the test
    def get_test_type_name(self):
        return self.get_child_text_by_name('TestTypeName')

    ## get the timeout defined for the test
    def get_timeout(self):
        return self.get_child_text_by_name('Timeout')

    ## get the test description
    def get_description(self):
        return self.get_child_text_by_name('Description')

#---------------------------------------------#
# User Local User Config File                 #
#---------------------------------------------#
## This class represents the user configuration file of MTV
class XMLMTVUserConfig(XMLEntity):
    ## calls the XMLEntity constructor.
    #  In addition, if an empty object is created, some of the required fields
    #  are created as well.
    def __init__(self, xml_file = None, schema_file = None, doc_root = None):
        XMLEntity.__init__(self, xml_file, schema_file, doc_root)
        if self._root is None:
            self._root = etree.Element('MTVUserConfig')
            self._root.append(etree.Element('MTVRepositoryUrl'))
            self._root.append(etree.Element('MTVProductReleaseDir'))
            self._root.append(etree.Element('MTVUserTempDir'))
            self._root.append(etree.Element('MTVClipdbRootDir'))
            self._root.append(etree.Element('ShowUserConfigMenuOnStartup'))
            self._root.append(etree.Element('VisualStudioDevenvDir'))
            self._root.append(etree.Element('IntelCompilerDir'))

    ## get the URL of the repository containing MTV
    def get_mtv_repository_url(self):
        return self.get_child_text_by_name('MTVRepositoryUrl')
    ## set the URL of the repository containing MTV
    #  @param path string: the MTV repository URL
    def set_mtv_repository_url(self, path):
        self.set_child_text_by_name('MTVRepositoryUrl', path)

    ## get the path to the MTV release area
    def get_mtv_product_release_dir(self):
        return self.get_child_text_by_name('MTVProductReleaseDir')
    ## set the path to the MTV release area
    #  @param rel_dir string: the path to the MTV release area
    def set_mtv_product_release_dir(self, rel_dir):
        self.set_child_text_by_name('MTVProductReleaseDir', rel_dir)

    ## get the user temporary directory in the NFS
    def get_mtv_user_temp_dir(self):
        return self.get_child_text_by_name('MTVUserTempDir')
    ## set the user temporary directory in the NFS
    #  @param temp_dir string: the path to the user temporary directory
    def set_mtv_user_temp_dir(self, temp_dir):
        self.set_child_text_by_name('MTVUserTempDir', temp_dir)

    ## get the path to the MTV clips database root directory
    def get_mtv_clipdb_root_dir(self):
        return self.get_child_text_by_name('MTVClipdbRootDir')
    ## set the path to the MTV clips database root directory
    #  @param root_dir string: the path to the clips DB root directory
    def set_mtv_clipdb_root_dir(self, root_dir):
        self.set_child_text_by_name('MTVClipdbRootDir', root_dir)

    ## get an indicator that determines whether to display the configuration
    #  dialog when MTV GUI starts.
    #  @return True if the dialog needs to be displayed, False otherwise
    def get_show_user_config_menu_on_startup(self):
        return self.get_child_text_by_name('ShowUserConfigMenuOnStartup')
    ## set the indicator that determines whether to display the configuration
    #  dialog when MTV GUI starts.
    #  @param value boolean: True when the dialog needs to be displayed, False otherwise.
    def set_show_user_config_menu_on_startup(self, value):
        self.set_child_text_by_name('ShowUserConfigMenuOnStartup', value)

    ## get the path to the visual studio directory, where Devenv.exe is found.
    def get_visual_studio_devenv_dir(self):
        return self.get_child_text_by_name('VisualStudioDevenvDir')
    ## set the path to the visual studio directory.
    #  @param devenv_dir string: the path to the visual studio directory, containing
    #         the Devenv.exe executable file.
    def set_visual_studio_devenv_dir(self, devenv_dir):
        self.set_child_text_by_name('VisualStudioDevenvDir', devenv_dir)

    ## get the path to the intel compiler directory
    def get_intel_compiler_dir(self):
        return self.get_child_text_by_name('IntelCompilerDir')
    ## set the path to the intel compiler directory.
    #  @param devenv_dir string: the path to the visual studio directory, containing
    #         the Devenv.exe executable file.
    def set_intel_compiler_dir(self, devenv_dir):
        self.set_child_text_by_name('IntelCompilerDir', devenv_dir)

    ## get the path to the last used local work directory
    def get_last_work_dir(self):
        return self.get_child_text_by_name('LastLocalWorkDir')
    ## set the path to the last used local work directory
    #  @param local_work_dir string: the path to the last used local
    #         work directory.
    def set_last_work_dir(self, local_work_dir):
        self.set_child_text_by_name('LastLocalWorkDir', local_work_dir)

    ## get the name of the product that the last shell was opened for
    def get_last_opened_product(self):
        return self.get_child_text_by_name('LastOpenedProduct')
    ## set the name of the product that the last shell was opened for
    #  @param last_opened_product string: the name of the last opened product
    def set_last_opened_product(self, last_opened_product):
        self.set_child_text_by_name('LastOpenedProduct', last_opened_product)

    ## get the name of the versions that the last shell was opened for
    def get_last_opened_version(self):
        return self.get_child_text_by_name('LastOpenedVersion')
    ## set the name of the version that the last shell was opened for
    #  @param last_opened_version string: the name of the last opened version
    def set_last_opened_version(self, last_opened_version):
        self.set_child_text_by_name('LastOpenedVersion', last_opened_version)

    ## get the indicator that determines if the shell is opened in off-line mode.
    #  If the shell is opened in off-line mode, there is no connection to the release
    #  area and to the SVN.
    #  @return True if the shell needs to be opened in off-line mode, False otherwise.
    def get_offline_mode(self):
        return self.get_child_text_by_name('OfflineMode')
    ## set the indicator that determines if the shell is opened in off-line mode.
    #  if the shell is opened in off-line mode, there is no connection to the release
    #  area and to the SVN.
    #  @param offline_mode boolean: the indicator value
    def set_offline_mode(self, offline_mode):
        self.set_child_text_by_name('OfflineMode', offline_mode)

#---------------------------------------------#
# Database Connection                         #
#---------------------------------------------#
## This class represents a database connection.
#  The database connection contains the host address for the database,
#  the user name and password and the database name.
class XMLMTVDatabaseConnection(XMLEntity):
    ## calls the XMLEntity constructor
    def __init__(self, xml_file = None, schema_file = None, doc_root = None):
        XMLEntity.__init__(self, xml_file, schema_file, doc_root)

    ## get the database host (machine name contianing the DB)
    def get_host(self):
        return self.get_child_text_by_name("Host")
    ## get the user name for the database login
    def get_user_name(self):
        return self.get_child_text_by_name("UserName")
    ## get the password for the database login
    def get_password(self):
        return self.get_child_text_by_name("Password")
    ## get he database name
    def get_db(self):
        return self.get_child_text_by_name("DB")


#---------------------------------------------#
# Database Configuration                      #
#---------------------------------------------#
## This class represents a database configuration.
#  The database configuration contains at least one database connection.
class XMLMTVDatabaseConfig(XMLEntity):
    ## calls the XMLEntity constructor
    def __init__(self, xml_file = None, schema_file = None, doc_root = None):
        XMLEntity.__init__(self, xml_file, schema_file, doc_root)
        self.parse()

    ## parse the XML file and extracts the data into local variables
    def parse(self):
        self._Connections = []

        connections = self.get_children_by_name("Connection")
#        if connections is not None:
        for connection in connections:
            self._Connections.append(XMLMTVDatabaseConnection(doc_root=connection))

    ## check if the database configuration has connections defined.
    def has_connections(self):
        return len(self._Connections) >0
    ## get a list of database connections of tpye XMLMTVDatabaseConnection.
    def get_connections(self):
        return self._Connections

#---------------------------------------------#
# Server Configuration                        #
#---------------------------------------------#
## This class represents an MTV server configuration
class XMLMTVServerConfig(XMLEntity):
    ## calls the XMLEntity constructor
    def __init__(self, xml_file = None, schema_file = None, doc_root = None):
        XMLEntity.__init__(self, xml_file, schema_file, doc_root)

    ## get the server hostname (machine name)
    def get_hostname(self):
        return self.get_child_text_by_name("HostName")
    ## get the server port number, on which it listens
    def get_port_number(self):
        return self.get_child_text_by_name("PortNumber")
    ## get the URL for the MTV repository.
    #  This value is the same value as defined in the user configuration file,
    #  but the server has no access to the user configuration file, and therefore
    #  it uses a separate configuration value for that.
    def get_mtv_repo_path(self):
        return self.get_child_text_by_name("MTVRepoPath")


#---------------------------------------------#
# Executer Configuration                      #
#---------------------------------------------#
## This class represents an MTV executer configuration
class XMLMTVExecuterConfig(XMLEntity):
    ## calls the XMLEntity constructor
    def __init__(self, xml_file = None, schema_file = None, doc_root = None):
        XMLEntity.__init__(self, xml_file, schema_file, doc_root)

    ## get the executer port number, on which it listens
    def get_port_number(self):
        return self.get_child_text_by_name("PortNumber")
    ## get the path to the executer base directory.
    #  This directory will contain all the files being checked-out by the executer
    #  and all the task and test executions and results, executed by the executer.
    def get_base_dir(self):
        return self.get_child_text_by_name("BaseDir")
    ## get indicator that determines if the executer is enabled and can be used to
    #  execute tasks, or disabled and cannot execute tasks.
    def is_enabled(self):
        enabled = self.get_attribute_by_name("Enabled")
        if enabled is not None:
            if lower(enabled) == "false":
                return False
        return True
    ## get the machine configuration of the executer machine.
    #  @return XML object of type XMLMachine
    def get_machine(self):
        machine = self.get_child_by_name("Machine")
        return XMLMachine(doc_root = machine)
    ## get the executer hostname (the machine name)
    def get_hostname(self):
        machine = self.get_machine()
        return machine.get_hostname()




#---------------------------------------------#
# System Configuration                        #
#---------------------------------------------#
## This class represents the MTV configuration XML file.
class XMLMTVConfig(XMLEntity):
    ## calls the XMLEntity constructor
    def __init__(self, xml_file = None, schema_file = None, doc_root = None):
        XMLEntity.__init__(self, xml_file, schema_file, doc_root)

    ## get the database configuration
    def get_database(self):
        database = self.get_child_by_name("Database")
        return XMLMTVDatabaseConfig(doc_root = database)
    ## get the server configuration
    def get_server(self):
        server_config = self.get_child_by_name("Server")
        return XMLMTVServerConfig(doc_root = server_config)
    ## get a list of the executers defined in the configuration.
    #  each executer is of type XMLMTVExecuterConfig
    def get_executers(self):
        executers_list = []
        executers = self.get_children_by_name("Executer")
        if (executers):
            for executer in executers:
                executers_list.append(XMLMTVExecuterConfig(doc_root = executer))
        return executers_list
    ## get a single executer from the configuration, by its name.
    #  This function uses a name resolving function (passed as parameter) that
    #  takes host names and return IP. This way it makes sure that the correct
    #  host is returned, regardless of possible name aliasing.
    #
    #  @param name string: the name of the required executer
    #  @param resolve_name function(string): a function that gets a string containing
    #         the required executer name and return its address (IP?)
    #
    #  @return a single executer of type XMLMTVExecuterConfig, or None if the
    #          executer with the given name is not defined in the MTV configuration.
    def get_executer_by_name(self, name, resolve_name):
        """
        gets the host name, and a function used to resolve the name into IP address.
        the comparison of host names is done by converting it to IP and comparing IP addresses.
        """
        executers_list = self.get_executers()

        for executer in executers_list:
            exec_name = executer.get_hostname()
            if resolve_name(name) == resolve_name(exec_name):
                return executer
        return None

#---------------------------------------------------------#
# Repository information in a ProductVersion              #
#---------------------------------------------------------#
## This class represents a repository defined inside a version file.
class XMLProductVersionRepository(XMLEntity):
    ## calls the XMLEntity constructor
    def __init__(self, xml_file = None, schema_file = None, doc_root = None):
        XMLEntity.__init__(self, xml_file, schema_file, doc_root)

    ## get the repository tag used to check-out the version's components
    def get_tag(self):
        return self.get_child_text_by_name('Tag')

    ## get the repository name
    def get_name(self):
        return self.get_attribute_by_name('Name')

    ## set the tag of the repository, used to checkout the version's components
    def set_tag(self, tag):
        self.set_child_text_by_name('Tag', tag)

    ## set the repository name
    def set_name(self, name):
        self.set_attribute_by_name('Name', name)

    ## print the version file repository
    def xprint(self):
        print 'Name:', self.get_name()
        print 'Tag:', self.get_tag()

#---------------------------------------------------------#
# A Version of a Product                                  #
# (Stand alone or part of the Products Versions xml file) #
#---------------------------------------------------------#
## This class represents an MTV version file.
#  Version file contains data about the version, such as the
#  creation date, the different repositories' tags for the
#  components in the version, etc.
class XMLProductVersion(XMLEntity):
    ## calls the XMLEntity constructor.
    #  In addition, if an empty object is created, its root is created as well.
    def __init__(self, xml_file = None, schema_file = None, doc_root = None):
        XMLEntity.__init__(self, xml_file, schema_file, doc_root)
        if self._root is None:
            self._root = etree.Element('ProductVersion')

    ## get the version description.
    #  This usually contain information on the promotion date etc.
    def get_description(self):
        return self.get_child_text_by_name('Description')

    ## get the version name
    def get_name(self):
        return self.get_attribute_by_name('Name')

    ## get the creation date of the version
    def get_date(self):
        return self.get_attribute_by_name('Date')

    ## get a list of the repositories defined for this version.
    #  each repository is of type XMLProductVersionRepository.
    def get_repositories(self):
        repo_list = []
        repositories = self.get_children_by_name("Repository")
        if (repositories):
            for repo in repositories:
                repo_list.append(XMLProductVersionRepository(doc_root = repo))
        return repo_list

    ## get one of the repositories defined in the version.
    #  @param name string: the repository name
    #  @return an XMLProductVersionRepository object, or None if a repository
    #          with the given name is not defined in the version file.
    def get_repository_by_name(self, name):
        repo_list = self.get_repositories()

        for repo in repo_list:
            if name == repo.get_name():
                return repo
        return None

    ## get the tag of the repository with the given name
    #  @param name string: the repository name
    #  @return the given repository's tag, or None if a repository
    #          with the given name is not defined in the version file.
    def get_repository_tag_by_name(self, name):
        repo = self.get_repository_by_name(name)

        if repo is not None:
            return repo.get_tag()

        return None

    ## set the version name
    #  @param name string: the repository name
    def set_name(self, name):
        self.set_attribute_by_name('Name', name)

    ## set the version creation date
    #  @param date string: the creation date
    def set_date(self, date):
        self.set_attribute_by_name('Date', date)

    ## set the version's description
    #  @param description string: the version's description
    def set_description(self, description):
        self.set_child_text_by_name('Description', description)

    ## set the repository tag, or add the repository to the version
    #  if the repository is not defined for the version.
    def set_repository(self, repo_name, tag):
        repo = self.get_repository_by_name(repo_name)

        if repo is not None:
            repo.set_tag(tag)
        else:
            repo = self.add_child_by_name('Repository')
            xml_repo = XMLProductVersionRepository(doc_root = repo)
            xml_repo.set_name(repo_name)
            xml_repo.set_tag(tag)

    ## print the version file
    def xprint(self):
        print 'Name:', self.get_name()
        print 'Date:', self.get_date()
        print 'Description:', self.get_description()
        repositories = self.get_repositories()
        for repo in repositories:
            print 'Repository:'
            print '-----------------------------------------------------------'
            repo.xprint()

#---------------------------------------------------------#
# A Version History of a Product File                     #
#---------------------------------------------------------#

class XMLProductVersions(XMLEntity):
    def __init__(self, xml_file = None, schema_file = None, doc_root = None):
        XMLEntity.__init__(self, xml_file, schema_file, doc_root)

    def get_name(self):
        return self.get_attribute_by_name('Name')

    def get_versions(self):
        ver_list = []
        versions = self.get_children_by_name("ProductVersion")
        if (versions):
            for ver in versions:
                ver_list.append(XMLProductVersion(doc_root = ver))
        return ver_list

    def xprint(self):
        print 'Name:', self.get_name()
        versions = self.get_versions()
        for ver in versions:
            print 'ProductVersion:'
            print '------------------------------------------------------------'
            ver.xprint()

#---------------------------------------------#
# Products History Versions File              #
#---------------------------------------------#
class XMLProductsVersions(XMLEntity):
    def __init__(self, xml_file = None, schema_file = None, doc_root = None):
        XMLEntity.__init__(self, xml_file, schema_file, doc_root)

    def get_products(self):
        prod_list = []
        products = self.get_children_by_name("Product")
        if (products):
            for prod in products:
                prod_list.append(XMLProductVersions(doc_root = prod))
        return prod_list

    def get_product_by_name(self, name):
        products = self.get_products()
        for prod in products:
            if prod.get_name() == name:
                return prod

    def xprint(self):
        products = self.get_products()
        for prod in products:
            print 'Product:'
            print '---------------------------------------------------------------'
            prod.xprint()



## This class represents a build configuration of the promotion test
class XMLPromotionBuildConfiguration(XMLEntity):
    ## calls the XMLEntity constructor
    def __init__(self, xml_file = None, schema_file = None, doc_root = None):
        XMLEntity.__init__(self, xml_file, schema_file, doc_root)

    ## get the build configuration OS type
    def get_os_type(self):
        return self.get_child_text_by_name('OSType')

    ## get the build configuration architecture
    def get_arch_type(self):
        return self.get_child_text_by_name('Architecture')

    ## get the build configuration for the compiler (i.e. Debug|Win32)
    def get_configuration(self):
        return self.get_child_text_by_name('Configuration')



## this class represents the local promotion configuration file for the product
#  This file contains several build configurations that are used to build the
#  product in all the available configurations on every machine.
class XMLPromotion(XMLEntity):
    ## calls the XMLEntity constructor
    def __init__(self, xml_file = None, schema_file = None, doc_root = None):
        XMLEntity.__init__(self, xml_file, schema_file, doc_root)

    ## get list of build configurations.
    #  each build configuraiton is of type XMLPromotionBuildConfiguration.
    def get_build_configs(self):
        build_configs_list = []
        build_configs = self.get_children_by_name('BuildConfiguration')

        for config in build_configs:
            build_configs_list.append(XMLPromotionBuildConfiguration(doc_root = config))

        return build_configs_list

    ## get a subset of the defined build configurations.
    #  This subset has the given OS type and architecture.
    #  This function is used to get the relevant build configurations
    #  for every machine trying to execute the promotion test.
    def get_build_configs_by_os_and_arch(self, os_type, arch_type):
        configurations = []
        build_configs = self.get_build_configs()

        for config in build_configs:
            if config.get_os_type() == os_type:
                if config.get_arch_type() == arch_type:
                    configurations.append(config.get_configuration())
        return configurations





def main():
    #dummy = XMLProductSDF('C:\\work\\hive_work\\hive\\boaz2\\src\\hive_prod\\hive_sdf.xml', 'c:\\work\\mtv\\src\\map\\mtv_schema.xsd')
    dummy = XMLProductsVersions('C:\\work\\test.xml', 'c:\\work\\mtv\\src\\map\\mtv_schema.xsd')
    dummy.xprint()

if __name__ == '__main__':
    main()
