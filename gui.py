#!/usr/bin/env python3
"""IITPSx assembler and emulator GUI; run with ``python3 gui.py``.

Declaration: This is my own work, written from scratch.
"""
import os
import re
import subprocess
import tkinter as tk
from tkinter import filedialog

# Keep these formats in sync with the emulator's -trace and HALT output.
TRACE_RE = re.compile(r"([0-9A-Fa-f]{8})\s+A=(-?\d+)\s+B=(-?\d+)\s+SP=(-?\d+)\s*:\s*(\S+)\s+(-?\d+)")
HALT_RE = re.compile(r"HALT:\s*A=(-?\d+)\s+B=(-?\d+)\s+PC=(-?\d+)\s+SP=(-?\d+)\s+steps=(\d+)")
MONO = "Courier"
BASE_DIR = os.path.dirname(os.path.abspath(__file__))


def project_path(path):
    """Resolve a GUI path against the project, not the launch directory."""
    if not os.path.isabs(path):
        path = os.path.join(BASE_DIR, path)
    return os.path.abspath(path)


def sh(args):
    # Use argv lists and a fixed project cwd so paths work from any launch folder.
    try:
        r = subprocess.run(args, cwd=BASE_DIR, capture_output=True, text=True, timeout=30)
        return r.returncode, r.stdout + r.stderr
    except Exception as e:
        return 1, str(e)


class App:
    """Editor and debugger controls backed by the C assembler and emulator."""

    def __init__(self, root):
        self.root = root
        root.title("IITPSx Assembler / Emulator")
        root.geometry("1160x740")
        self.cur = project_path("tests/test04.asm")
        self.trace = []
        self.halt = ""
        self.idx = 0
        self.pcmap = {}
        self._menu()
        self._toolbar()
        self._panes()
        self.ensure_built()
        if os.path.isfile(self.cur):
            self.load_file(self.cur)

    def _menu(self):
        m = tk.Menu(self.root)
        f = tk.Menu(m, tearoff=0)
        f.add_command(label="Open...", command=self.do_open)
        f.add_command(label="Save", command=self.do_save)
        f.add_separator()
        f.add_command(label="Exit", command=self.root.destroy)
        m.add_cascade(label="File", menu=f)
        b = tk.Menu(m, tearoff=0)
        b.add_command(label="Assemble", command=self.do_asm)
        b.add_command(label="Run", command=self.do_run)
        b.add_command(label="Step", command=self.do_step)
        b.add_command(label="Reset", command=self.do_reset)
        b.add_separator()
        b.add_command(label="Dump memory", command=self.do_dump)
        b.add_command(label="Run all tests", command=self.do_runall)
        m.add_cascade(label="Build", menu=b)
        self.root.config(menu=m)

    def _toolbar(self):
        t = tk.Frame(self.root, bd=1, relief="raised")
        t.pack(fill="x")
        self.fentry = tk.Entry(t, width=30, font=MONO)
        self.fentry.insert(0, self.cur)
        self.fentry.pack(side="left", padx=4, pady=3)
        for txt, cmd in (("Open", self.do_open), ("Save", self.do_save),
                         ("Assemble", self.do_asm), ("Run", self.do_run),
                         ("Step", self.do_step), ("Reset", self.do_reset),
                         ("Dump", self.do_dump), ("Run all tests", self.do_runall)):
            tk.Button(t, text=txt, command=cmd).pack(side="left", padx=2, pady=3)
        self.root.bind("<F5>", lambda e: self.do_asm())
        self.root.bind("<F6>", lambda e: self.do_run())
        self.root.bind("<F7>", lambda e: self.do_step())

    def _panes(self):
        main = tk.PanedWindow(self.root, orient="horizontal", sashwidth=5)
        main.pack(fill="both", expand=True)
        left = tk.PanedWindow(main, orient="vertical", sashwidth=5)
        main.add(left, minsize=600)
        right = tk.Frame(main, width=340)
        main.add(right)
        # code editor
        lf = tk.LabelFrame(left, text="Code")
        left.add(lf, minsize=200)
        self.ed = tk.Text(lf, font=MONO, undo=True, bd=1, relief="sunken")
        sb = tk.Scrollbar(lf, command=self.ed.yview)
        self.ed.config(yscrollcommand=sb.set)
        sb.pack(side="right", fill="y")
        self.ed.pack(fill="both", expand=True)
        # listing
        lf2 = tk.LabelFrame(left, text="Listing")
        left.add(lf2, minsize=120)
        self.lst = tk.Text(lf2, font=MONO, bd=1, relief="sunken")
        sb2 = tk.Scrollbar(lf2, command=self.lst.yview)
        self.lst.config(yscrollcommand=sb2.set)
        sb2.pack(side="right", fill="y")
        self.lst.pack(fill="both", expand=True)
        self.lst.tag_config("cur", background="#316ac5", foreground="white")
        # registers
        rf = tk.LabelFrame(right, text="Registers")
        rf.pack(fill="x", padx=3, pady=3)
        self.regs = {}
        for k in ("A", "B", "PC", "SP", "STEP"):
            row = tk.Frame(rf)
            row.pack(fill="x", padx=4, pady=1)
            tk.Label(row, text=k, width=5, anchor="w", font=MONO).pack(side="left")
            v = tk.Label(row, text="0", anchor="e", font=MONO, bd=1, relief="sunken", width=14)
            v.pack(side="right")
            self.regs[k] = v
        self.set_regs(0, 0, 0, 0, 0)
        # current instruction
        inf = tk.LabelFrame(right, text="Instruction")
        inf.pack(fill="x", padx=3, pady=3)
        self.curins = tk.Label(inf, text="-", font=MONO, anchor="w")
        self.curins.pack(fill="x", padx=4, pady=3)
        # memory
        mf = tk.LabelFrame(right, text="Memory (addr: word)")
        mf.pack(fill="both", expand=True, padx=3, pady=3)
        self.mem = tk.Text(mf, font=MONO, bd=1, relief="sunken")
        sb3 = tk.Scrollbar(mf, command=self.mem.yview)
        self.mem.config(yscrollcommand=sb3.set)
        sb3.pack(side="right", fill="y")
        self.mem.pack(fill="both", expand=True)
        # output
        of = tk.LabelFrame(self.root, text="Output")
        of.pack(fill="x", padx=3, pady=3)
        btns = tk.Frame(of)
        btns.pack(side="right", fill="y")
        tk.Button(btns, text="Clear", command=lambda: self.out.delete("1.0", "end")).pack(fill="x", padx=3, pady=2)
        self.out = tk.Text(of, height=8, font=MONO, bd=1, relief="sunken")
        sb4 = tk.Scrollbar(of, command=self.out.yview)
        self.out.config(yscrollcommand=sb4.set)
        sb4.pack(side="right", fill="y")
        self.out.pack(fill="both", expand=True)

    def log(self, s):
        self.out.insert("end", s + "\n")
        self.out.see("end")

    def ensure_built(self):
        # Build lazily so opening the GUI does not require prebuilt executables.
        if os.path.isfile(project_path("asm")) and os.path.isfile(project_path("emu")):
            return True
        self.log("building asm and emu ...")
        rc, out = sh(["make"])
        if rc != 0:
            rc, out = sh(["make", "asm", "emu"])
        if rc == 0 and os.path.isfile(project_path("asm")) and os.path.isfile(project_path("emu")):
            self.log("build ok")
            return True
        self.log("build failed:")
        for ln in out.strip().splitlines():
            self.log("  " + ln)
        return False

    def set_regs(self, a, b, pc, sp, st):
        self.regs["A"].config(text=str(a))
        self.regs["B"].config(text=str(b))
        self.regs["PC"].config(text=str(pc))
        self.regs["SP"].config(text=str(sp))
        self.regs["STEP"].config(text=str(st))

    def load_file(self, p):
        p = project_path(p)
        try:
            with open(p, "r", encoding="utf-8") as source:
                contents = source.read()
            self.ed.delete("1.0", "end")
            self.ed.insert("1.0", contents)
            self.cur = p
            self.fentry.delete(0, "end")
            self.fentry.insert(0, p)
        except Exception as e:
            self.log(str(e))

    def do_open(self):
        f = filedialog.askopenfilename(initialdir=BASE_DIR,
                                       filetypes=[("asm", "*.asm"), ("all", "*.*")])
        if f:
            self.load_file(f)

    def do_save(self):
        p = project_path(self.fentry.get().strip() or self.cur)
        try:
            with open(p, "w", encoding="utf-8", newline="") as source:
                source.write(self.ed.get("1.0", "end-1c"))
            self.cur = p
            self.fentry.delete(0, "end")
            self.fentry.insert(0, p)
            return True
        except Exception as e:
            self.log(str(e))
            return False

    def do_asm(self):
        if not self.do_save():
            return
        if not self.ensure_built():
            return
        src = self.cur
        rc, out = sh([project_path("asm"), src,
                      project_path("out.bin"), project_path("out.lst")])
        self.lst.delete("1.0", "end")
        if rc == 0:
            try:
                with open(project_path("out.lst"), "r", encoding="utf-8") as listing:
                    txt = listing.read()
                self.lst.insert("1.0", txt)
                self.build_pcmap(txt)
            except Exception as e:
                self.log(str(e))
            self.log(f"{src}: assembled ok, {os.path.getsize(project_path('out.bin'))//4} words -> out.bin, out.lst")
            self.preload_trace()
            self.do_dump(silent=True)
        else:
            self.log(f"{src}: assembly failed")
            self.log(out.strip())
            if os.path.exists(project_path("out.bin")):
                try:
                    os.remove(project_path("out.bin"))
                except Exception:
                    pass

    def build_pcmap(self, lsttxt):
        # Map instruction addresses to listing rows for current-step highlighting.
        self.pcmap = {}
        for i, ln in enumerate(lsttxt.splitlines()):
            m = re.match(r"([0-9A-Fa-f]{8})\s+[0-9A-Fa-f]{8}", ln)
            if m:
                self.pcmap[int(m.group(1), 16)] = i

    def preload_trace(self):
        # Cache one bounded execution trace so stepping does not rerun every click.
        self.trace, self.halt = [], ""
        if not os.path.exists(project_path("out.bin")):
            return
        rc, out = sh([project_path("emu"), "-trace", "-steps", "500000",
                      project_path("out.bin")])
        for ln in out.splitlines():
            if TRACE_RE.match(ln):
                self.trace.append(ln)
                if len(self.trace) >= 500000:
                    break
            elif ln.startswith("HALT"):
                self.halt = ln
        self.idx = 0
        self.set_regs(0, 0, 0, 0, 0)
        self.curins.config(text="-")

    def show_step(self):
        self.lst.tag_remove("cur", "1.0", "end")
        if self.idx < len(self.trace):
            m = TRACE_RE.match(self.trace[self.idx])
            if m:
                pc = int(m.group(1), 16)
                a, b, sp = int(m.group(2)), int(m.group(3)), int(m.group(4))
                mn, op = m.group(5), m.group(6)
                self.set_regs(a, b, pc, sp, self.idx)
                self.curins.config(text=f"PC={pc} {mn} {op}")
                if pc in self.pcmap:
                    ln = self.pcmap[pc] + 1
                    self.lst.tag_add("cur", f"{ln}.0", f"{ln}.end")
                    self.lst.see(f"{ln}.0")
        elif self.halt:
            m = HALT_RE.match(self.halt)
            if m:
                self.set_regs(m.group(1), m.group(2), m.group(3), m.group(4), m.group(5))
                self.curins.config(text=self.halt)
                self.log(self.halt)

    def do_step(self):
        if not os.path.exists(project_path("out.bin")):
            self.log("assemble first")
            return
        if not self.trace:
            self.preload_trace()
        if self.idx < len(self.trace):
            self.show_step()
            self.idx += 1
            if self.idx == len(self.trace) and self.halt:
                self.show_step()
        else:
            self.log("end of trace")

    def do_reset(self):
        self.idx = 0
        self.set_regs(0, 0, 0, 0, 0)
        self.curins.config(text="-")
        self.lst.tag_remove("cur", "1.0", "end")

    def do_run(self):
        if not self.ensure_built():
            return
        if not os.path.exists(project_path("out.bin")):
            self.log("assemble first")
            return
        rc, out = sh([project_path("emu"), project_path("out.bin")])
        self.log(out.strip())
        m = HALT_RE.search(out)
        if m:
            self.set_regs(m.group(1), m.group(2), m.group(3), m.group(4), m.group(5))
            self.curins.config(text=m.group(0))
        self.do_dump(silent=True)

    def do_dump(self, silent=False):
        if not os.path.exists(project_path("out.bin")):
            if not silent:
                self.log("assemble first")
            return
        dumpfile = os.path.join(BASE_DIR, "iitps_dump.txt")
        rc, out = sh([project_path("emu"), "-dump", dumpfile,
                      project_path("out.bin")])
        try:
            with open(dumpfile, "r", encoding="utf-8") as dump:
                lines = dump.read().splitlines()[:128]
            self.mem.delete("1.0", "end")
            self.mem.insert("1.0", "\n".join(lines))
        except Exception as e:
            if not silent:
                self.log(str(e))

    def do_runall(self):
        # Each case gets separate outputs so it cannot overwrite the open program.
        if not self.ensure_built():
            return
        d = project_path("tests") if os.path.isdir(project_path("tests")) else BASE_DIR
        files = sorted(f for f in os.listdir(d) if f.endswith(".asm"))
        self.log("=" * 46)
        self.log("running all test files")
        self.log("=" * 46)
        for f in files:
            src = project_path(os.path.join(d, f))
            test_bin = project_path("test-run.bin")
            test_lst = project_path("test-run.lst")
            rc, out = sh([project_path("asm"), src, test_bin, test_lst])
            if rc != 0:
                self.log(f"[{src}] assemble FAILED")
                for ln in out.strip().splitlines():
                    self.log("  " + ln)
                continue
            nwords = os.path.getsize(test_bin) // 4
            self.log(f"[{src}] assembled ok ({nwords} words)")
            test_dump = project_path("test-run.dump")
            rc2, out2 = sh([project_path("emu"), "-dump", test_dump,
                            "-steps", "500000", test_bin])
            for ln in out2.strip().splitlines():
                self.log("  " + ln)
            if f == "bubble.asm":
                try:
                    with open(test_dump, "r", encoding="utf-8") as dump:
                        words = [int(line.split()[1], 16) for line in dump]
                    arr = words[1:6]
                    self.log(f"  sorted array: {arr}")
                except Exception:
                    pass


if __name__ == "__main__":
    r = tk.Tk()
    App(r)
    r.mainloop()
