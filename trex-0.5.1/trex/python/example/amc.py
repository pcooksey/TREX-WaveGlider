#! /usr/bin/python

import trex

class log_printer(trex.utils.log_handler):
    "A simple trex log handler that print log messages"
    def __init__(self):
        "constructor required to have this code not resulting on a crash at destruction"
        trex.utils.log_handler.__init__(self)
    def new_entry(self, e):
        "the handling code itself print the entry into python standard output"
        "note: this is done within a C++ thread with correct python thread protection"
        date=''
        if( e.is_dated ):
            date = '[{}]'.format(e.date())
        print '{}[{}]{}: {}'.format(date, e.source(), e.kind(), e.content())

trex_log = log_printer();

def run_agent(file_name):
    "batch execution of an agent"
    "This call executes an agent based on the config sepcified in the file file_name"
    agent = trex.agent.agent(file_name)
    default_clock = trex.agent.rt_clock(1000)
    agent.set_clock(default_clock)
    agent.run()
    

