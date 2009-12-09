#############################################################################
# $Id:$
# Test Name:                                                                #
#    Regression Test Script for ASDM "version 0.9"  import to MS            #
#                                                                           #
# Rationale for Inclusion:                                                  #
#    The conversion of ASDM to MS needs to be verified.                     #
#                                                                           # 
# Features tested:                                                          #
#    1) Is the import performed without raising exceptions                  #
#    2) Do all expected tables exist                                        #
#    3) Can the MS be opened                                                #
#    4) Do the tables contain expected values                               #
#                                                                           #
# Input data:                                                               #
#     one dataset for the old filler (ASDM "0.9")                           #
#                                                                           #
#############################################################################

myname = 'asdm-import_regression'

# default ASDM dataset name
myasdm_dataset_name = 'uid___X54_X1418_X1'

# get the dataset name from the wrapper if possible
mydict = locals()
if mydict.has_key("asdm_dataset_name"):
    myasdm_dataset_name = mydict["asdm_dataset_name"]

# name of the resulting MS
msname = myasdm_dataset_name+'.ms'

def checktable(thename, theexpectation):
    global msname, myname
    tb.open(msname+"/"+thename)
    if thename == "":
        thename = "MAIN"
    for mycell in theexpectation:
        print myname, ": comparing ", mycell
        value = tb.getcell(mycell[0], mycell[1])
        # see if value is array
        try:
            isarray = value.__len__
        except:
            # it's not an array
            # zero tolerance?
            if mycell[3] == 0:
                in_agreement = (value == mycell[2])
            else:
                in_agreement = ( abs(value - mycell[2]) < mycell[3]) 
        else:
            # it's an array
            # zero tolerance?
            if mycell[3] == 0:
                in_agreement =  (value == mycell[2]).all() 
            else:
                in_agreement = (abs(value - mycell[2]) < mycell[3]).all() 
        if not in_agreement:
            print myname, ":  Error in MS subtable", thename, ":"
            print "     column ", mycell[0], " row ", mycell[1], " contains ", value
            print "     expected value is ", mycell[2]
            tb.close()
            raise
    tb.close()
    print myname, ": table ", thename, " as expected."
    return

try:
    importoldasdm(myasdm_dataset_name)
except:
    print myname, ": Error ", sys.exc_info()[0]
    raise
else:
    print myname, ": Success! Now checking output ..."
    mscomponents = set(["table.dat",
                        "table.f0",
                        "table.f1",
                        "table.f2",
                        "table.f3",
                        "table.f4",
                        "table.f5",
                        "table.f6",
                        "table.f7",
                        "table.f8",
                        "ANTENNA/table.dat",
                        "DATA_DESCRIPTION/table.dat",
                        "FEED/table.dat",
                        "FIELD/table.dat",
                        "FLAG_CMD/table.dat",
                        "HISTORY/table.dat",
                        "OBSERVATION/table.dat",
                        "POINTING/table.dat",
                        "POLARIZATION/table.dat",
                        "PROCESSOR/table.dat",
                        "SOURCE/table.dat",
                        "SPECTRAL_WINDOW/table.dat",
                        "STATE/table.dat",
                        "SYSCAL/table.dat",
                        "ANTENNA/table.f0",
                        "DATA_DESCRIPTION/table.f0",
                        "FEED/table.f0",
                        "FIELD/table.f0",
                        "FLAG_CMD/table.f0",
                        "HISTORY/table.f0",
                        "OBSERVATION/table.f0",
                        "POINTING/table.f0",
                        "POLARIZATION/table.f0",
                        "PROCESSOR/table.f0",
                        "SOURCE/table.f0",
                        "SPECTRAL_WINDOW/table.f0",
                        "STATE/table.f0",
                        "SYSCAL/table.f0"
                        ])
    for name in mscomponents:
        if not os.access(msname+"/"+name, os.F_OK):
            print myname, ": Error  ", msname+"/"+name, "doesn't exist ..."
            raise
        else:
            print myname, ": ", name, "present."
    print myname, ": MS exists. All tables present. Try opening as MS ..."
    try:
        ms.open(msname)
    except:
        print myname, ": Error  Cannot open MS table", tablename
        raise
    else:
        ms.close()
        print myname, ": OK. Checking tables in detail ..."

        # check main table first
        name = ""
        #             col name, row number, expected value, tolerance
        expected = [ ['UVW',       42, [1.9006977, -22.538636, 26.703018], 1E-6],
                     ['EXPOSURE',  42, 1.008, 0],
                     ['DATA',      42, [ [0.00025587+0.00000797j] ], 1E-8]
                     ]
        checktable(name, expected)
        
        name = "ANTENNA"
        expected = [ ['OFFSET',       1, [ 0.,  0.,  0.], 0],
                     ['POSITION',     1, [-1601361.760577, -5042192.535329,  3554531.519329], 0],
                     ['DISH_DIAMETER',1, 12.0, 0]
                     ]
        checktable(name, expected)
        
        name = "POINTING"
        expected = [ ['DIRECTION',       10, [[0.958],[0.416791]], 1E-6 ],
                     ['INTERVAL',        10, 0.048, 0],
                     ['TARGET',          10, [[ 0.95722],[ 0.4183]], 1E-6 ],
                     ['TIME',            10, 4735409708, 1],
                     ['TIME_ORIGIN',     10, 0., 0],
                     ['POINTING_OFFSET', 10, [[ 0.],[ 0.]], 0],
                     ['ENCODER',         10, [0.958002, 0.416791], 1E-6 ]
                     ]
        checktable(name, expected)
        


        
        
    

