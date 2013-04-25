"""
The flagdatasetter module interfaces hid heuristic flaggers to CASA flagdata.
"""
from __future__ import absolute_import
import os
import types

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
from pipeline.infrastructure import casa_tasks

# the logger for this module
LOG = infrastructure.get_logger(__name__)


class FlagdataSetterInputs(basetask.StandardInputs):
    """
    FlagdataSetter manages the inputs for the FlagdataSetter task.
    """    

    def __init__(self, context, vis=None, output_dir=None, inpfile=None,
                 reason=None): 
        """
        Initialise the Inputs, initialising any property values to those given
        here.
        
        :param context: the pipeline Context state object
        :type context: :class:`~pipeline.infrastructure.launcher.Context`
        :param vis: the measurement set(s) to flag
        :type vis: a string or list of strings
        :param output_dir: the output directory for pipeline data
        :type output_dir: string
        :param inpfile: file with flagcmds
        :type inpfile: string
        :param reason: string specifying subset of flagcmds to apply
        :type reason: string
        """        
        # set the properties to the values given as input arguments
        self._init_properties(vars())

    @property
    def inpfile(self):
        if type(self.vis) is types.ListType:
            return self._handle_multiple_vis('inpfile')
        
        if self._inpfile is None:
            vis_root = os.path.splitext(self.vis)[0]
            return '%s_flagcmds.txt' % vis_root
        return self._inpfile

    @inpfile.setter
    def inpfile(self, value):
        self._inpfile = value
    
    @property
    def reason(self):
        if self._reason is None:
            return ''
        return self._reason
    
    @reason.setter
    def reason(self, value):
        self._reason = value        


class FlagdataSetterResults(basetask.Results):
    def __init__(self, jobs=[]):
        """
        Initialise the results object with the given list of JobRequests.
        """
        super(FlagdataSetterResults, self).__init__()
        self.jobs = jobs

    def __repr__(self):
        s = 'flagdata results:\n'
        for job in self.jobs:
            s += '%s performed.' % str(job)
        return s 


class FlagdataSetter(basetask.StandardTaskTemplate):
    """
    """
    Inputs = FlagdataSetterInputs

    def prepare(self):
        """
        Prepare and execute a flagdata flagging job appropriate to the
        task inputs.
        """
        inputs = self.inputs
        
        # to save inspecting the file, also log the flag commands
#        LOG.debug('Flag commands for %s:\n%s' % (inputs.vis, flag_cmds))

        # create and execute a flagdata job
        job = casa_tasks.flagdata(vis=inputs.vis, mode='list', action='apply',
          inpfile=inputs.inpfile, savepars=False, flagbackup=False,
          reason=inputs.reason)
        self._executor.execute(job)

        return FlagdataSetterResults([job])

    def analyse(self, results):
        """
        Analyse the results of the flagging operation.
        """
        return results

    def flags_to_set(self, flags):
        """
        Add flags to template file.
        """
        inputs = self.inputs
        with open(inputs.inpfile, 'a') as stream:
            for flag in flags:
                stream.write('%s\n' % flag.flagcmd)
                inputs.reason = flag.reason

