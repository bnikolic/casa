"""
Code in the argmapper module transforms CASA input arguments to pipeline
inputs arguments.
  
CASA arguments may be named differently to their corresponding arguments in
the pipeline classes. This module defines how those arguments should be
renamed so that they are acceptable to the Inputs constructors, and secondly
it converts from the CASA concept of null values ('', [], etc.) to the 
pipeline equivalent.
"""
from __future__ import absolute_import
import inspect
import types

from . import logging
import pipeline

LOG = logging.get_logger(__name__)


# _altmapping holds the mapping for CASA arguments that should be given as a 
# task input argument of a different name.
_altmapping = {
#    'Bandpass'      : {'hm_bandtype' : 'mode'},
    'Gaincal'       : {'hm_gaintype' : 'mode'},
#    'TimeGaincal'   : {'hm_gaintype' : 'mode'},
#    'PhcorBandpass' : {'hm_bandtype' : 'mode'}
}


def convert_args(taskname, casa_args, convert_nulls=True):
    """
    Convert CASA arguments to pipeline Inputs arguments.
    
    This function converts a dictionary of CASA arguments to the corresponding
    dictionary of pipeline Inputs arguments by doing the following: 
    
    #. Rename CASA arguments to their pipeline equivalents.
    #. Remove any arguments not accepted by the Inputs constructor.
    #. Convert any CASA null values to pipeline null values.
    
    :param taskname: the task name
    :type taskname: string 
    :param casa_args: the dictionary of CASA arguments and values
    :type casa_args: dict
    :rtype: dict
    """
    # Get the task class, and from that the Inputs class 
    task_cls = getattr(pipeline.tasks, taskname)
    inputs_cls = task_cls.Inputs

    # If required, rename CASA pipeline arguments to their pipeline equivalent
    if taskname in _altmapping:
        mapping = _altmapping[taskname]
        remapped = {}
        for k, v in casa_args.iteritems():
            name = mapping.get(k, k)
            if k != name and name in casa_args:
                msg = ('Remapping %s for %s inputs overwrites an argument of '
                       'the same name' % (name, taskname))
                LOG.error(msg)
                raise AttributeError, msg
            remapped[name] = v
    else:
        remapped = casa_args

    LOG.debug('Original arguments: %s' % casa_args)
    LOG.debug('Remapped arguments: %s' % remapped)
    
    # inspect the signature of the Inputs constructor, getting a list of the
    # keyword arguments accepted by the constructor
    try:
        constructor_args = inputs_cls.get_constructor_args()
    except AttributeError:
        constructor_args = inspect.getargspec(inputs_cls.__init__).args

    # remove any arguments that are not accepted by the input constructor
    accepted = {}
    accepted.update((k,v) for k, v in remapped.iteritems()
                    if k in constructor_args)

    # convert any CASA empty string or empty lists to None, thus allowing the
    # pipeline to use its default value.
    if convert_nulls:
        converted = {}
        converted.update((k, _convert_null(v)) for k, v in accepted.items())
    else:
        converted = accepted

    LOG.debug('Final arguments for %s: %s' % (taskname, converted))
    return converted

def _convert_null(val):
    # convert empty string to None
    if type(val) is types.StringType:
        return None if val in ['', 'context'] else val

    # convert empty lists and ['', ''] etc to None
    if type(val) is types.ListType:
        no_nulls = [_convert_null(v) for v in val]
        nones = [t==None for t in no_nulls]
        if all(nones):
            return None
        else:
            return no_nulls

    return val

def task_to_casa(taskname, task_args):
    if taskname not in _altmapping:
        return task_args
    
    # If required, rename CASA pipeline arguments to their pipeline equivalent
    casa_to_task = _altmapping[taskname]
    d = dict((v,k) for k, v in casa_to_task.iteritems())
    remapped = {}
    for k, v in task_args.iteritems():
        name = d.get(k, k)
        remapped[name] = v

    return remapped

def inputs_to_casa(inputs_cls, args):
    for mod_entry in dir(pipeline.tasks):
        task = getattr(pipeline.tasks, mod_entry)
        task_inputs = getattr(task, 'Inputs', None)
        if task_inputs == inputs_cls.__class__:        
            return task_to_casa(mod_entry, args)
    return args