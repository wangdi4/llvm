from optparse import OptionParser
import sys
import csv

# RawCounter is container of list with specific size (1 for regular counters and > 1 for complex counters)
# The list will store the value in index 'i' (for primitives i = 0) of specific counter
class RawCounter(object):

    def __init__(self, amount):
        self.__amount = amount
        self.__values = list()
        # Nullify the values
        for i in range (0,amount):
            self.__values.append(0)

    def getAmount(self):
        if self.__amount <> len(self.__values):
            print 'Error: list size is different than req. amount'
            sys.exit(1)
        return self.__amount

    def getValueAt(self, index):
        if (index >= self.__amount):
            print 'Error: Wrong index'
            sys.exit(1)
        return self.__values[index]

    def setValueAt(self, index, value):
        if (index >= self.__amount):
            print 'Error: Wrong index'
            sys.exit(1)
        self.__values[index] = value

# RawCommand represent command counters, it contains map from counter name to RawCounter
class RawCommand(object):

    def __init__(self, countersNameToNumOfInstancesMap):
        self.__counterNameToValue = dict()
        for name, amount in countersNameToNumOfInstancesMap.iteritems():
            self.__counterNameToValue[name] = RawCounter(amount)

    def getNames(self):
        return self.__counterNameToValue.keys()

    def getAmount(self, counterName):
        if False == counterName in self.__counterNameToValue:
            print 'Error: Counter name does not exist'
            sys.exit(1)
        return self.__counterNameToValue[counterName].getAmount()

    def getValueAt(self, counterName, index):
        if False == counterName in self.__counterNameToValue:
            print 'Error: Counter name does not exist'
            sys.exit(1)
        return self.__counterNameToValue[counterName].getValueAt(index)

    def setValueAt(self, counterName, index, value):
        if False == counterName in self.__counterNameToValue:
            print 'Error: Counter name does not exist'
            sys.exit(1)
        self.__counterNameToValue[counterName].setValueAt(index, value)
            
class RawTraceParser(object):

    # self._counters will be map< pair< counter_name, num_of_counters_included >, list< RawCommandCounters >>
    def __init__(self):
        # Map of counter name to num of instances for this name
        self.__countersNameToNumOfInstances = dict()
        # map from command ID to all the counters of this command
        self.__commandIdToRawCommand = dict()
        # command IDs list (In order to save the order)
        self.__commandIdList = list()
        self.__hostFreq = 0
        self.__deviceFreq = 0
        # configuration list (each item include pair of (title, value)
        self.__configurationList = list()

    def parseHeaders(self, input1, input2):
        if False == self.__openInputFiles(input1, input2):
            return False
        # read host header
        self.__hostFreq = self.__readHeader(self.__hostFile) / 1000.0
        # read device header
        self.__deviceFreq = self.__readHeader(self.__deviceFile) / 1000.0
        return True

    def loadInputValues(self):
        self.__readInput(self.__hostFile, self.__hostFreq)
        self.__readInput(self.__deviceFile, self.__deviceFreq)

    def getCountersNames(self):
        return self.__countersNameToNumOfInstances.keys()

    def getNumOfInstances(self, counterName):
        return self.__countersNameToNumOfInstances[counterName]

    def getOrderCommandIDs(self):
        return self.__commandIdList

    def getRawCommand(self, commandID):
        return self.__commandIdToRawCommand[commandID]

    def getConfigurationList(self):
        return self.__configurationList
            

    def __openInputFiles(self, input1, input2):
        try:
            if (input1.find("host") >= 0) and (input2.find("device") >= 0):
                self.__hostFile = open(input1, 'r')
                self.__deviceFile = open(input2, 'r')
            elif (input1.find("device") >= 0) and (input2.find("host") >= 0):
                self.__deviceFile = open(input1, 'r')
                self.__hostFile = open(input2, 'r')
            else:
                print 'Error: The input files name should include \'host\' and \'device\' in their name'
                return False
        except IOError as e:
            print(e)
            return False
        return True

    # Read and store the headers data, assume that the file handler point to the beginning of the file
    def __readHeader(self, fileHeader):
        freq = 0
        line = fileHeader.readline()
        readFreq = False
        readCounters = False
        readConfiguration = False
        while (line <> '') and (line.find("End header") == -1):
            if readFreq:
                freq = long(line)
                readFreq = False
                continue
            if line.find("COUNTERS}") <> -1:
                readCounters = False
            if readCounters:
                commandName = line.split('[')[0].replace('\t', '')
                numOfItems = int(line.split('[')[1].split(']')[0])
                self.__countersNameToNumOfInstances[commandName] = numOfItems
            if line.find("CONFIGURATION}") <> -1:
                readConfiguration = False
            if readConfiguration:
                currConfPair = line.lstrip('\t').split('\t')
                if len(currConfPair) == 2:
                    self.__configurationList.append(currConfPair)
            if line.find("{FREQUENCY") <> -1:
                readFreq = True
            if line.find("{COUNTERS") <> -1:
                readCounters = True
            if line.find("{CONFIGURATION") <> -1:
                readConfiguration = True
            line = fileHeader.readline()
        return freq

    # read and store the input counters values, assume that the file handler point one line after the header.
    # It also calculate the time of each timer by deviding by frequency.
    # in - inputFile, freq.
    def __readInput(self, inputFile, freq):
        if (None == inputFile) or (0 == freq):
            return False
        line = inputFile.readline()
        # while not end of file
        currentCommandData = dict()
        while line <> '':
            if line.find("{") <> -1:
                line = inputFile.readline()
                while (line <> '') and (line.find("}") == -1):
                    commandName = line.split('[')[0].replace('\t', '')
                    index = int(line.split('[')[1].split(']')[0])
                    valueStr = line.split(']')[1].replace('\t', '').replace('\n', '')
                    if valueStr.isdigit():
                        if commandName.find("time") <> -1:
                            # save the value and the freq and do not divide it here in order to have better precision.
                            value = (long(valueStr), freq)
                        else:
                            value = int(valueStr)
                    else:
                        value = valueStr
                    if False == (commandName in currentCommandData):
                        currentCommandData[commandName] = list()
                    currentCommandData[commandName].append((index, value))
                    line = inputFile.readline()
                if len(currentCommandData.keys()) > 0:
                    # each command must have command_id
                    if False == "command_id" in currentCommandData:
                        print "Error: No command_id found in command."
                        return False
                    cmdId = currentCommandData["command_id"][0][1]
                    # if this command_id already exist, shall extend the command counters, otherwise create it first
                    if False == (cmdId in self.__commandIdToRawCommand):
                        # push the command_id to the end of the list in order to save the commands order
                        self.__commandIdList.append(cmdId)
                        # Initialize new command
                        self.__commandIdToRawCommand[cmdId] = RawCommand(self.__countersNameToNumOfInstances)
                    # Traverse over currentCommandData
                    for name, indexValuePairList in currentCommandData.iteritems():
                        for index, value in indexValuePairList:
                            self.__commandIdToRawCommand[cmdId].setValueAt(name, index, value)
                currentCommandData.clear()

            line = inputFile.readline()
        return True      

class PrintedTraceParser(object):

    def __init__(self, parsedRawData):
        defineCounters = ("Command type",
                          "Command ID",
                          "Serialization time",
                          "Deserialziation time",
                          "Num of buffer operations",
                          "Buffer operations - Buffers size",
                          "Num buffers sent to NDRange",
                          "NDRange buffers size",
                          "Kernel name",
                          "Global work size in dimension",
                          "Work group size in dimenstion",
                          "Command preperation time",
                          "COI enqueue to COI notification",
                          "R.T. execution after notification",
                          "Command overall time",
                          "Device - NDRange PreExecution time",
                          "Device - NDRange PostExecution time",
                          "Device - NDRange TBB execution time",
                          "Device - NDRange overall time",
                          "Host to Device & Device to Host trasfer time",
                          "Device - Num of invocations by thread",
                          "Device - Num of WG executed by thread",
                          "Device - Overall time for thread")
        
        self._orderCountersNames = list()
        for name in defineCounters:
            self._orderCountersNames.append(name)
                                    
        self._countersToFunc = {defineCounters[0] : (self.__simple, 1, "command_type"),
                                defineCounters[1] : (self.__simple, 1, "command_id"),
                                defineCounters[2] : (self.__delta_with_freq_dev, 1, "build_serialize_time_start", "build_serialize_time_end"),
                                defineCounters[3] : (self.__delta_with_freq_dev, 1, "build_deserialize_time_start", "build_deserialize_time_end"),
                                defineCounters[4] : (self.__simple, 1, "num_of_buffer_operations"),
                                defineCounters[5] : (self.__simple, 1, "buffer_operation_overall_size"),
                                defineCounters[6] : (self.__simple, 1, "num_of_buffer_sent_to_device"),
                                defineCounters[7] : (self.__simple, 1, "buffers_size_sent_to_device"),
                                defineCounters[8] : (self.__simple, 1, "kernel_name"),
                                defineCounters[9] : (self.__simple, 3, "global_work_size"),
                                defineCounters[10] : (self.__simple, 3, "work_group_size"),
                                defineCounters[11] : (self.__delta_with_freq_dev, 1, "command_host_time_start", "coi_execute_command_time_start"),
                                defineCounters[12] : (self.__delta_with_freq_dev, 1, "coi_execute_command_time_start", "coi_execute_command_time_end"),
                                defineCounters[13] : (self.__delta_with_freq_dev, 1, "coi_execute_command_time_end", "command_host_time_end"),
                                defineCounters[14] : (self.__delta_with_freq_dev, 1, "command_host_time_start", "command_host_time_end"),
                                defineCounters[15] : (self.__delta_with_freq_dev, 1, "cmd_run_in_device_time_start", "tbb_exe_in_device_time_start"),
                                defineCounters[16] : (self.__delta_with_freq_dev, 1, "tbb_exe_in_device_time_end", "cmd_run_in_device_time_end"),
                                defineCounters[17] : (self.__delta_with_freq_dev, 1, "tbb_exe_in_device_time_start", "tbb_exe_in_device_time_end"),
                                defineCounters[18] : (self.__delta_with_freq_dev, 1, "cmd_run_in_device_time_start", "cmd_run_in_device_time_end"),
                                defineCounters[19] : (self.__delta, 1, "COI enqueue to COI notification", "Device - NDRange overall time"),
                                defineCounters[20] : (self.__simple, parsedRawData.getNumOfInstances("thread_num_of_invocations"), "thread_num_of_invocations"),
                                defineCounters[21] : (self.__simple, parsedRawData.getNumOfInstances("thread_num_wg_exe"), "thread_num_wg_exe"),
                                defineCounters[22] : (self.__dev_by_freq, parsedRawData.getNumOfInstances("thread_overall_time"), "thread_overall_time")}

        # set of use raw counters
        counterNamesSet = set()
        for counterName in self._countersToFunc:
            for rawCounterName in self._countersToFunc[counterName][2:]:
                counterNamesSet.add(rawCounterName)

        additionalCounters = list()
        for counterName in parsedRawData.getCountersNames():
            if False == (counterName in counterNamesSet):
                additionalCounters.append(counterName)
        #Add raw counters that does not exist in "Hard coded" counters
        for counterName in additionalCounters:
             self._orderCountersNames.append(counterName)
             self._countersToFunc[counterName] = (self.__simple, parsedRawData.getNumOfInstances(counterName), counterName)
            
                
        # list of command counters which represent by list of counter result which present as list (cmd1(counter1(val[0], val[1], ...), counter2(val[0], val[1], ...), ...), cmd2...)
        self.__commandsCountersValues = list()

        self.__parseRawData = parsedRawData

    def __getCounterValue(self, rawCommand, name, index):
        #if counter in RawCommand take its value
        if name in rawCommand.getNames():
            return rawCommand.getValueAt(name, index)
        #else if it is counter which not calculated as raw data
        elif name in self._countersToFunc.keys():
            return self.__processCounter(rawCommand, name, index)
        else:
            print "Error: Counter does not exist"
            sys.exit(1)


    # In __delta paramsList will be (start, end)
    # start - The counter name that represent the beginning time
    # end - The counter name that represent the end time.
    # index - the index of start and end in thier vector.
    def __delta(self, rawCommand, index, paramsList):
        if len(paramsList) <> 2:
            print "Error: __delta_with_freq_dev shold have 2 parameters in paramsList"
            sys.exit(1)
        startName, endName = paramsList
        start =  self.__getCounterValue(rawCommand, startName, index)
        end = self.__getCounterValue(rawCommand, endName, index)
        if (start == 0) or (end == 0):
            return 0
        return (start - end)

    # In __dev_by_freq paramsList will be (time)
    # this method will divide time by freq
    # time - The counter name that represent the time
    # index - the index of start and end in thier vector.
    def __dev_by_freq(self, rawCommand, index, paramsList):
        if len(paramsList) <> 1:
            print "Error: __delta_with_freq_dev shold have 2 parameters in paramsList"
            sys.exit(1)
        timeName = paramsList[0]
        timeWithFreq = self.__getCounterValue(rawCommand, timeName, index)
        if (isinstance(timeWithFreq, (list, tuple))) and (len(timeWithFreq) == 2):
            time = timeWithFreq[0]
            freq = timeWithFreq[1]
            return float(time) / float(freq)
        else:
            return int(0)

    # In __delta_with_freq_dev paramsList will be (start, end)
    # this method will subtract end with start and divide by freq
    # start - The counter name that represent the beginning time
    # end - The counter name that represent the end time.
    # index - the index of start and end in thier vector.
    def __delta_with_freq_dev(self, rawCommand, index, paramsList):
        if len(paramsList) <> 2:
            print "Error: __delta_with_freq_dev shold have 2 parameters in paramsList"
            sys.exit(1)
        startName, endName = paramsList
        startWithFreq = self.__getCounterValue(rawCommand, startName, index)
        endWithFreq = self.__getCounterValue(rawCommand, endName, index)
        if (isinstance(startWithFreq, (list, tuple))) and (isinstance(endWithFreq, (list, tuple))) and (len(startWithFreq) == 2) and (len(endWithFreq) == 2):
            start = startWithFreq[0]
            freqStart = startWithFreq[1]
            end = endWithFreq[0]
            freqEnd = endWithFreq[1]
            if freqStart <> freqEnd:
                print "Error: Not corelate frequencies"
                sys.exit(1)
            return float(end - start) / float(freqStart)
        else:
            return int(0)
        
        
    # In __simple paramsList will be (counter)
    # counter - The counter name that represent counter
    # index - the index of start and end in thier vector.
    def __simple(self, rawCommand, index, paramsList):
        if len(paramsList) <> 1:
            print "Error: __delta_with_freq_dev shold have 2 parameters in paramsList"
            sys.exit(1)
        return self.__getCounterValue(rawCommand, paramsList[0], index)

    def __processCounter(self, currentRawCommand, name, index):
        if False == (name in self._countersToFunc):
            print "Error: Counter does not exist in _countersToFunc"
            sys.exit(1)
        return self._countersToFunc[name][0](currentRawCommand, index, self._countersToFunc[name][2:])

    def __processCommandCounters(self, currentRawCommand):
        countersList = list()
        for commandName in self._orderCountersNames:
            counterValues = list()
            for index in range (0, self._countersToFunc[commandName][1]):
                counterValues.append(self.__processCounter(currentRawCommand, commandName, index))
            countersList.append(counterValues)
        return countersList

    def processAllCommandsCounters(self):
        orderCommandIDs = self.__parseRawData.getOrderCommandIDs()
        for commandID in orderCommandIDs:
            currentRawCommand = self.__parseRawData.getRawCommand(commandID)
            self.__commandsCountersValues.append(self.__processCommandCounters(currentRawCommand))

#        for command in self.__commandsCountersValues:
#            index = 0
#            print "******************************"
#            for counter in command:
#                name = self._orderCountersNames[index]
#                valIndex = 0
#                for value in counter:
#                    print name + "[" + str(valIndex) + "] = " + str(value)
#                    valIndex  = valIndex + 1
#                index = index + 1
#                    
#        print self._orderCountersNames
#        print self.__commandsCountersValues

    def writeToCsvFile(self, fileName):
        try:
            outFile = open(fileName, 'wb')
        except IOError as e:
            print(e)
            return False
        csvWriter = csv.writer(outFile)

        # Write the configuration to the 2 first raws of the trace csv file
        confList = self.__parseRawData.getConfigurationList()
        for i in range (0, 2):
            configRaw = list()
            for p in confList:
                configRaw.append(p[i].rstrip('\n'))
            csvWriter.writerow(configRaw)

        complexCounterIndex = 0
        threadsCountersContiniuesAmount = 1
        counterIndex = 0
        while counterIndex < len(self._orderCountersNames):
            currentRaw = list()
            # if simple counter (with one instance)
            currentRaw.append(self._orderCountersNames[counterIndex])
            if self._countersToFunc[self._orderCountersNames[counterIndex]][1] == 1:
                currentRaw.append("")
            elif self._countersToFunc[self._orderCountersNames[counterIndex]][1] > 1:
                currentRaw.append(complexCounterIndex)
            for command in self.__commandsCountersValues:
                currentRaw.append("")
                # if simple counter (with one instance)
                if self._countersToFunc[self._orderCountersNames[counterIndex]][1] == 1:
                    currentRaw.append(command[counterIndex][0])
                # if counter with more than one instance
                elif self._countersToFunc[self._orderCountersNames[counterIndex]][1] > 1:
                    currentRaw.append(command[counterIndex][complexCounterIndex])
            if self._countersToFunc[self._orderCountersNames[counterIndex]][1] > 1:
                # if counter of threads takes the next counters of threads and print them in the followin order:
                # counter1[0]
                # counter2[0]
                # ...
                # counter1[1]
                # counter2[1]
                # ...
                if self._orderCountersNames[counterIndex].find("thread") <> -1:
                    # If the next <> 'thread counter' and...
                    if ((counterIndex == (len(self._orderCountersNames) - 1)) or (self._orderCountersNames[counterIndex + 1].find("thread") == -1)):
                        # and We didn't write all the instances
                        if complexCounterIndex < (self._countersToFunc[self._orderCountersNames[counterIndex]][1] - 1):
                            counterIndex = counterIndex - threadsCountersContiniuesAmount
                            threadsCountersContiniuesAmount = 1
                        # and We write all the instances
                        elif complexCounterIndex == (self._countersToFunc[self._orderCountersNames[counterIndex]][1] - 1):
                            complexCounterIndex = 0
                            threadsCountersContiniuesAmount = 1
                            counterIndex  = counterIndex + 1
                            csvWriter.writerow(currentRaw)
                            continue
                        complexCounterIndex = complexCounterIndex + 1
                    else:
                        threadsCountersContiniuesAmount = threadsCountersContiniuesAmount + 1
                # if complex counter that have more than one instance and not 'thread counter'
                else:
                    if complexCounterIndex == (self._countersToFunc[self._orderCountersNames[counterIndex]][1] - 1):
                        complexCounterIndex = 0
                        counterIndex = counterIndex + 1
                        csvWriter.writerow(currentRaw)
                        continue
                    counterIndex = counterIndex - 1
                    complexCounterIndex = complexCounterIndex + 1

            counterIndex = counterIndex + 1
            csvWriter.writerow(currentRaw)

        outFile.close()
        
        





def main():

    parser = OptionParser(usage = 'uasge: %prog [options]', description = "Tracer post processing")
    parser.add_option("-i", "--input", dest = "input_traces", help = "Enter list of input tracer files", default = '')
    parser.add_option("-o", "--output", dest = "output_trace", help = "Enter name for the output file", default = '')

    (options, args) = parser.parse_args()

    inFiles = options.input_traces.split(',')
    outFile = options.output_trace
    if (outFile <> '') and (False == outFile.endswith(".csv")):
        outFile = outFile + ".csv"

    if len(inFiles) <> 2:
        print 'Error: Please enter 2 input files'
        sys.exit(1)
    if outFile == '':
        outFile = 'trace_out.csv'

    rawParser = RawTraceParser()
    rawParser.parseHeaders(inFiles[0], inFiles[1])
    rawParser.loadInputValues()

    printParser = PrintedTraceParser(rawParser)
    printParser.processAllCommandsCounters()
    printParser.writeToCsvFile(outFile)

main()
