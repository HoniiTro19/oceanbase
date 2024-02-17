# coding: utf-8
# OceanBase Deploy.
# Copyright (C) 2021 OceanBase
#
# This file is part of OceanBase Deploy.
#
# OceanBase Deploy is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# OceanBase Deploy is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with OceanBase Deploy.  If not, see <https://www.gnu.org/licenses/>.

from __future__ import absolute_import, division, print_function

from stdio import IO

import os
import sys
import textwrap
from uuid import uuid1 as uuid
from optparse import OptionParser, BadOptionError, Option, IndentedHelpFormatter

from core import TestBench
from stdio import IO
from tool import DirectoryUtil
from error import LockError

ROOT_IO = IO(1)


class OptionHelpFormatter(IndentedHelpFormatter):
    def format_option(self, option):
        result = []
        opts = self.option_strings[option]
        opt_width = self.help_position - self.current_indent - 2
        if len(opts) > opt_width:
            opts = "%*s%s\n" % (self.current_indent, "", opts)
            indent_first = self.help_position
        else:  # start help on same line as opts
            opts = "%*s%-*s  " % (self.current_indent, "", opt_width, opts)
            indent_first = 0
        result.append(opts)
        if option.help:
            help_text = self.expand_default(option)
            help_lines = help_text.split("\n")
            if len(help_lines) == 1:
                help_lines = textwrap.wrap(help_text, self.help_width)
            result.append("%*s%s\n" % (indent_first, "", help_lines[0]))
            result.extend(
                ["%*s%s\n" % (self.help_position, "", line) for line in help_lines[1:]]
            )
        elif opts[-1] != "\n":
            result.append("\n")
        return "".join(result)


class AllowUndefinedOptionParser(OptionParser):
    IS_TTY = sys.stdin.isatty()

    def __init__(
        self,
        usage=None,
        option_list=None,
        option_class=Option,
        version=None,
        conflict_handler="error",
        description=None,
        formatter=None,
        add_help_option=True,
        prog=None,
        epilog=None,
        allow_undefine=True,
        undefine_warn=True,
    ):
        OptionParser.__init__(
            self,
            usage,
            option_list,
            option_class,
            version,
            conflict_handler,
            description,
            formatter,
            add_help_option,
            prog,
            epilog,
        )
        self.allow_undefine = allow_undefine
        self.undefine_warn = undefine_warn

    def warn(self, msg, file=None):
        if self.IS_TTY:
            print("%s %s" % (IO.WARNING_PREV, msg))
        else:
            print("warn: %s" % msg)

    def _process_long_opt(self, rargs, values):
        try:
            value = rargs[0]
            OptionParser._process_long_opt(self, rargs, values)
        except BadOptionError as e:
            if self.allow_undefine:
                key = e.opt_str
                value = value[len(key) + 1 :]
                setattr(
                    values,
                    key.strip("-").replace("-", "_"),
                    value if value != "" else True,
                )
                self.undefine_warn and self.warn(e)
            else:
                raise e

    def _process_short_opts(self, rargs, values):
        try:
            value = rargs[0]
            OptionParser._process_short_opts(self, rargs, values)
        except BadOptionError as e:
            if self.allow_undefine:
                key = e.opt_str
                value = value[len(key) + 1 :]
                setattr(
                    values,
                    key.strip("-").replace("-", "_"),
                    value if value != "" else True,
                )
                self.undefine_warn and self.warn(e)
            else:
                raise e


class BaseCommand(object):
    def __init__(self, name, summary):
        self.name = name
        self.summary = summary
        self.args = []
        self.cmds = []
        self.opts = {}
        self.prev_cmd = ""
        self.is_init = False
        self.hidden = False
        self.parser = AllowUndefinedOptionParser(add_help_option=False)
        self.parser.add_option(
            "-h",
            "--help",
            action="callback",
            callback=self._show_help,
            help="Show help and exit.",
        )
        self.parser.add_option(
            "-v",
            "--verbose",
            action="callback",
            callback=self._set_verbose,
            help="Activate verbose output.",
        )
        self.parser.add_option(
            "-t",
            "--traceid",
            default=str(uuid()),
            help="Identifier of a unique testbench command.",
        )

    def _set_verbose(self, *args, **kwargs):
        ROOT_IO.set_verbose_level(0xFFFFFFF)

    def init(self, cmd, args):
        if self.is_init is False:
            self.prev_cmd = cmd
            self.args = args
            self.is_init = True
            self.parser.prog = self.prev_cmd
            option_list = self.parser.option_list[2:]
            option_list.append(self.parser.option_list[0])
            option_list.append(self.parser.option_list[1])
            self.parser.option_list = option_list
        return self

    def parse_command(self):
        self.opts, self.cmds = self.parser.parse_args(self.args)
        return self.opts

    def do_command(self):
        raise NotImplementedError

    def _show_help(self, *args, **kwargs):
        ROOT_IO.print(self._mk_usage())
        self.parser.exit(1)

    def _mk_usage(self):
        return self.parser.format_help(OptionHelpFormatter())


class TestBenchCommand(BaseCommand):
    HOME_PATH = os.path.join(os.getenv("HOME"), ".testbench")

    def init_home(self):
        if os.path.exists(self.HOME_PATH):
            return
        for part in ["results", "log", "repository", "config"]:
            part_dir = os.path.join(self.HOME_PATH, part)
            DirectoryUtil.mkdir(part_dir)

    def parse_command(self):
        super(TestBenchCommand, self).parse_command()

    def do_command(self):
        self.parse_command()
        self.init_home()
        traceid = getattr(self.opts, "traceid", "")
        ret = False
        try:
            log_dir = os.path.join(self.HOME_PATH, "log")
            DirectoryUtil.mkdir(log_dir)
            log_path = os.path.join(log_dir, "testbench")
            ROOT_IO.init_trace_logger(log_path, "testbench", traceid)
            tb = TestBench(self.HOME_PATH, self.opts, ROOT_IO)
            ROOT_IO.track_limit += 1
            ROOT_IO.verbose("cmd: %s" % self.cmds)
            ROOT_IO.verbose("opts: %s" % self.opts)
            self._do_command(tb)
        except NotImplementedError:
            ROOT_IO.exception("command '%s' is not implemented" % self.prev_cmd)
        except LockError:
            ROOT_IO.exception("Another app is currently holding the obd lock.")
        except SystemExit:
            pass
        except KeyboardInterrupt:
            ROOT_IO.exception("Keyboard Interrupt")
        except:
            e = sys.exc_info()[1]
            ROOT_IO.exception("Running Error: %s" % e)
        ROOT_IO.print("Trace ID: %s" % traceid)
        return ret

    def _do_command(self, tb):
        raise NotImplementedError

    def _do_step(self, description, callback):
        ROOT_IO.start_loading(description)
        if callback():
            ROOT_IO.stop_loading("succeed")
        else:
            ROOT_IO.stop_loading("failed")
            raise RuntimeError


class MajorCommand(BaseCommand):
    def __init__(self, name, summary):
        super(MajorCommand, self).__init__(name, summary)
        self.commands = {}

    def _mk_usage(self):
        if self.commands:
            usage = ["%s <command> [options]\n\nAvailable commands:\n" % self.prev_cmd]
            commands = [x for x in self.commands.values()]
            commands.sort(key=lambda x: x.name)
            for command in commands:
                usage.append("%-14s %s\n" % (command.name, command.summary))
            self.parser.set_usage("\n".join(usage))
        return super(MajorCommand, self)._mk_usage()

    def do_command(self):
        if not self.is_init:
            ROOT_IO.error("%s command not init" % self.prev_cmd)
            raise SystemExit("command not init")
        if len(self.args) < 1:
            ROOT_IO.print(
                "You need to give some commands.\n\nTry `testbench --help` for more information."
            )
            self._show_help()
            return False
        base, args = self.args[0], self.args[1:]
        if base not in self.commands:
            self.parse_command()
            self._show_help()
            return False
        cmd = "%s %s" % (self.prev_cmd, base)
        ROOT_IO.track_limit += 1
        return self.commands[base].init(cmd, args).do_command()

    def register_command(self, command):
        self.commands[command.name] = command


class ClusterMajorCommand(MajorCommand):
    def __init__(self):
        super(ClusterMajorCommand, self).__init__(
            "cluster", "Manage a local cluster, and only one cluster can be deployed."
        )
        self.register_command(ClusterDeployCommand())
        self.register_command(ClusterDestroyCommand())
        self.register_command(ClusterDisplayCommand())
        self.register_command(ClusterCreateTenantCommand())


class ClusterDeployCommand(TestBenchCommand):
    def __init__(self):
        super(ClusterDeployCommand, self).__init__(
            "deploy", "Deploy a cluster with the given configuration file."
        )
        self.parser.add_option(
            "-c", "--config", type="string", help="Path to the configuration file."
        )
        self.parser.add_option(
            "-m",
            "--monitor",
            action="store_true",
            help="Monitor system status and alert abnormal behavior.",
        )

    def _check(self):
        config = getattr(self.opts, "config", "")
        if not config:
            ROOT_IO.error("Fail to deploy a cluster without configuration file.")
            return False
        if not os.path.exists(config):
            ROOT_IO.error("Configuration file {} does not exist.".format(config))
            return False
        return True

    def _do_command(self, tb):
        self._do_step("Checking options.", self._check)
        self._do_step("Deploying the local cluster.", tb.deploy_cluster)
        self._do_step("Starting the local cluster.", tb.start_cluster)
        self._do_step("Bootstraping the local cluster.", tb.bootstrap)
        self._do_step("Creating testbench tenant.", tb.create_tenant)


class ClusterCreateTenantCommand(TestBenchCommand):
    def __init__(self):
        super(ClusterCreateTenantCommand, self).__init__(
            "tenant", "Create testbench tenant with remained resources."
        )

    def _do_command(self, tb):
        self._do_step("Createing testbench tenant.", tb.create_tenant)


class ClusterDestroyCommand(TestBenchCommand):
    def __init__(self):
        super(ClusterDestroyCommand, self).__init__(
            "destroy", "Stop all servers and clear the workspace for each server."
        )

    def _do_command(self, tb):
        self._do_step("Destroying local cluster.", tb.destroy_cluster)


class ClusterDisplayCommand(TestBenchCommand):
    def __init__(self):
        super(ClusterDisplayCommand, self).__init__(
            "display", "Display the status of each server."
        )

    def _do_command(self, tb):
        self._do_step("Displaying the local cluster.", tb.display_cluster)


class BenchMajorCommand(MajorCommand):
    def __init__(self):
        super(BenchMajorCommand, self).__init__(
            "bench",
            "Start a new microbench with the given workload schedule configuration.",
        )
        self.register_command(BenchDataCommand())
        self.register_command(BenchLoadCommand())
        self.register_command(BenchTestCommand())
        self.register_command(BenchMocknetCommand())


class BenchDataCommand(TestBenchCommand):
    def __init__(self):
        super(BenchDataCommand, self).__init__(
            "data", "Generate and save data in the local storage."
        )
        self.parser.add_option(
            "-c", "--config", type="string", help="Path to the workload configuration file."
        )
        self.parser.add_option(
            "-f", "--force", action="store_true", default=False, help="Overwrite the existing databset."
        )
        
    def _check(self):
        config = getattr(self.opts, "config", "")
        if not config:
            ROOT_IO.error("Fail to generate dataset without configuration file.")
            return False

        if not os.path.exists(config):
            ROOT_IO.error("Configuration file {} does not exist.".format(config))
            return False
        return True

    def _do_command(self, tb):
        self._do_step("Generating datasets for the benchmark table.", tb.generate_dataset)


class BenchLoadCommand(TestBenchCommand):
    def __init__(self):
        super(BenchLoadCommand, self).__init__(
            "load", "Populate data into the cluster."
        )
        self.parser.add_option(
            "-c", "--config", type="string", help="Path to the workload configuration file."
        )
    
    def _check(self):
        config = getattr(self.opts, "config", "")
        if not config:
            ROOT_IO.error("Fail to generate dataset without configuration file.")
            return False

        if not os.path.exists(config):
            ROOT_IO.error("Configuration file {} does not exist.".format(config))
            return False
        return True

    def _do_command(self, tb):
        self._do_step("Generating datasets for the benchmark table.", tb.generate_dataset)
        self._do_step("Clearing schemas for the benchmark table.", tb.clear_schema)
        self._do_step("Creating schemas for the benchmark table.", tb.generate_schema)
        self._do_step("Loading datasets into the benchmark table.", tb.load_dataset)
        

class BenchTestCommand(TestBenchCommand):
    def __init__(self):
        super(BenchTestCommand, self).__init__(
            "test", "Start a benchmark process with the given configuration file."
        )
        self.parser.add_option(
            "-c", "--config", type="string", help="Path to the configuration file."
        )
        self.parser.add_option(
            "-e", "--elr", action="store_true", default=False, help="Need to use elr during benchmarking."
        )
        self.parser.add_option(
            "-l", "--lcl", action="store_true", default=False, help="Need to use lcl during benchmarking."
        )
    
    def _check(self):
        config = getattr(self.opts, "config", "")
        if not config:
            ROOT_IO.error("Fail to start transaction scheduler without configuration file.")
            return False

        if not os.path.exists(config):
            ROOT_IO.error("Configuration file {} does not exist.".format(config))
            return False
        return True

    def _do_command(self, tb):
        need_elr = getattr(self.opts, "elr", False)
        need_lcl = getattr(self.opts, "lcl", False)
        if need_elr:
            self._do_step("Enabling elr for contention transactions.", tb.enable_elr)
        if need_lcl:
            self._do_step("Enabling lcl for deadlock transactions.", tb.enable_lcl)
        self._do_step("Starting testbench scheduler.", tb.start_scheduler)
        if need_elr:
            self._do_step("Disabling elr for contention transactions.", tb.reset_elr)
        if need_lcl:
            self._do_step("Disabling lcl for deadlock transactions.", tb.reset_lcl)

class BenchMocknetCommand(TestBenchCommand):
    def __init__(self):
        super(BenchMocknetCommand, self).__init__(
            "mocknet", "Simulate the network environment among database instances."
        )
        self.parser.add_option(
            "-c", "--config", type="string", help="Path to the cluster configuration file."
        )
        self.parser.add_option(
            "-d", "--delay", type="int", help="Millisecond-level network latency."
        )
        self.parser.add_option(
            "-l", "--loss", type="int", help="Precentagewise network packet loss rate."
        )
    
    def _check(self):
        config = getattr(self.opts, "config", "")
        if not config:
            ROOT_IO.error("Fail to set mocknet without cluster configuration file.")
            return False
        if not os.path.exists(config):
            ROOT_IO.error("Configuration file {} does not exists.".format(config))
            return False
        
        loss = int(getattr(self.opts, "loss", "0"))
        if loss > 100 or loss < 0:
            ROOT_IO.error("Network packet loss rate get {}, which should be in the range [0, 100]".format(loss))
            return False
        return True
    
    def _do_command(self, tb):
        self._do_step("Simulating network environment.", tb.enable_mocknet)
        self._do_step("Reseting network environment.", tb.disable_mocknet)
        
class ReportMajorCommand(MajorCommand):
    def __init__(self):
        super(ReportMajorCommand, self).__init__(
            "report", "Generate statistic report for a benchmarking process."
        )


class MainCommand(MajorCommand):
    def __init__(self):
        super(MainCommand, self).__init__("testbench", "")
        self.register_command(ClusterMajorCommand())
        self.register_command(BenchMajorCommand())


if __name__ == "__main__":
    ROOT_IO.track_limit += 2
    if MainCommand().init("testbench", sys.argv[1:]).do_command():
        ROOT_IO.exit(0)
    ROOT_IO.exit(1)
