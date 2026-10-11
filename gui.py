#!/usr/bin/env python3
"""IITPSx Studio - MARS-like GUI, green theme. Run: python3 gui.py"""
import os
import re
import subprocess
import tkinter as tk
from tkinter import filedialog, messagebox

BG = "#0a140a"
PANEL = "#102410"
PANEL2 = "#14301a"
ACC = "#00e676"
ACC2 = "#69f0ae"
TXT = "#e8f5e9"
DIM = "#7cb342"
ERR = "#ff5252"
EDITBG = "#0d1f0d"
EDITFG = "#d7ffd7"

TRACE_RE = re.compile(r"([0-9A-Fa-f]{8})\s+A=(-?\d+)\s+B=(-?\d+)\s+SP=(-?\d+)\s*:\s*(\S+)\s+(-?\d+)")
HALT_RE = re.compile(r"HALT:\s*A=(-?\d+)\s+B=(-?\d+)\s+PC=(-?\d+)\s+SP=(-?\d+)\s+steps=(\d+)")

def sh(cmd):
    try:
        r = subprocess.run(cmd, shell=True, capture_output=True, text=True, timeout=15)
        return r.returncode, r.stdout + r.stderr
    except Exception as e:
        return 1, str(e)

class Studio:
    def __init__(self, root):
        self.root = root
        root.title("asm emu gui")
        root.configure(bg=BG)
        root.geometry("1180x760")
        self.cur = "test04.asm"
        self.trace = []
        self.halt = ""
        self.idx = 0
        self.pcmap = {}
        self._toolbar()
        self._main()
        if os.path.exists(self.cur):
            self.load_file(self.cur)

    def _btn(self, parent, txt, cmd):
        b = tk.Button(parent, text=txt, command=cmd, bg="#1b5e20", fg="white",
                      activebackground="#2e7d32", activeforeground="white",
                      relief="flat", padx=10, pady=4, font=("Consolas", 10, "bold"))
        b.pack(side="left", padx=3)
        return b

    def _toolbar(self):
        t = tk.Frame(self.root, bg="#0d3311", pady=6)
        t.pack(fill="x")
        self.fentry = tk.Entry(t, width=34, bg=EDITBG, fg=TXT, insertbackground=ACC)
        self.fentry.insert(0, self.cur)
        self.fentry.pack(side="left", padx=4)
        self._btn(t, "Open", self.do_open)
        self._btn(t, "Save", self.do_save)
        self._btn(t, "Assemble F5", self.do_asm)
        self._btn(t, "Run F6", self.do_run)
        self._btn(t, "Step F7", self.do_step)
        self._btn(t, "Reset", self.do_reset)
        self._btn(t, "Dump", self.do_dump)
        self.root.bind("<F5>", lambda e: self.do_asm())
        self.root.bind("<F6>", lambda e: self.do_run())
        self.root.bind("<F7>", lambda e: self.do_step())

    def _main(self):
        pan = tk.PanedWindow(self.root, orient="horizontal", bg=BG, sashwidth=4)
        pan.pack(fill="both", expand=True, padx=6, pady=6)
        left = tk.Frame(pan, bg=BG)
        pan.add(left, minsize=620)
        right = tk.Frame(pan, bg=BG, width=380)
        pan.add(right)
        # editor
        tk.Label(left, text="code", bg=BG, fg=ACC).pack(anchor="w")
        self.ed = tk.Text(left, height=20, bg=EDITBG, fg=EDITFG, insertbackground=ACC,
                          font=("Consolas", 11), undo=True)
        self.ed.pack(fill="both", expand=True)
        # listing
        tk.Label(left, text="listing", bg=BG, fg=ACC).pack(anchor="w")
        self.lst = tk.Text(left, height=12, bg="#0c1a0c", fg=ACC2, font=("Consolas", 10))
        self.lst.pack(fill="both", expand=True)
        self.lst.tag_config("cur", background="#1b5e20", foreground="white")
        # registers
        rf = tk.LabelFrame(right, text="regs", bg=PANEL, fg=ACC)
        rf.pack(fill="x", padx=2, pady=2)
        self.regs = {}
        for k in ("A", "B", "PC", "SP", "STEP"):
            row = tk.Frame(rf, bg=PANEL)
            row.pack(fill="x", padx=6, pady=1)
            tk.Label(row, text=k, width=6, anchor="w", bg=PANEL, fg=DIM,
                     font=("Consolas", 11, "bold")).pack(side="left")
            v = tk.Label(row, text="0", anchor="e", bg=PANEL2, fg=TXT,
                         font=("Consolas", 12, "bold"), width=14)
            v.pack(side="right")
            self.regs[k] = v
        self.set_regs(0, 0, 0, 65535, 0)
        # instr
        inf = tk.LabelFrame(right, text="instr", bg=PANEL, fg=ACC)
        inf.pack(fill="x", padx=2, pady=2)
        self.curins = tk.Label(inf, text="-", bg=PANEL, fg=TXT, font=("Consolas", 11))
        self.curins.pack(padx=6, pady=4)
        # memory
        tk.Label(right, text="memory", bg=BG, fg=ACC).pack(anchor="w")
        self.mem = tk.Text(right, height=14, bg="#0c1a0c", fg=ACC2, font=("Consolas", 10))
        self.mem.pack(fill="both", expand=True)
        # output
        bot = tk.Frame(self.root, bg=BG)
        bot.pack(fill="x", padx=6, pady=2)
        tk.Label(bot, text="Output", bg=BG, fg=ACC).pack(side="left")
        tk.Button(bot, text="Clear", command=lambda: self.out.delete("1.0", "end")).pack(side="right")
        self.out = tk.Text(self.root, height=6, bg="white", fg="black", font="TkFixedFont")
        self.out.pack(fill="x", padx=6, pady=(0, 6))

    def log(self, s, tag=None):
        self.out.insert("end", s + "\n")
        self.out.see("end")

    def set_regs(self, a, b, pc, sp, st):
        self.regs["A"].config(text=str(a))
        self.regs["B"].config(text=str(b))
        self.regs["PC"].config(text=str(pc))
        self.regs["SP"].config(text=str(sp))
        self.regs["STEP"].config(text=str(st))

    def load_file(self, p):
        try:
            self.ed.delete("1.0", "end")
            self.ed.insert("1.0", open(p).read())
            self.cur = p
            self.fentry.delete(0, "end")
            self.fentry.insert(0, p)
        except Exception as e:
            self.log(str(e), "err")

    def do_open(self):
        f = filedialog.askopenfilename(filetypes=[("asm", "*.asm"), ("all", "*.*")])
        if f:
            self.load_file(f)

    def do_save(self):
        p = self.fentry.get().strip() or self.cur
        try:
            open(p, "w").write(self.ed.get("1.0", "end-1c"))
            self.cur = p
            self.log("saved", "ok")
        except Exception as e:
            self.log(str(e), "err")

    def do_asm(self):
        self.do_save()
        src = self.cur
        rc, out = sh(f"./asm {src} out.bin out.lst")
        self.lst.delete("1.0", "end")
        try:
            txt = open("out.lst").read() if rc == 0 else ""
            self.lst.insert("1.0", txt)
            self.build_pcmap(txt)
        except Exception:
            pass
        if rc == 0:
            self.log("done", "ok")
            self.preload_trace()
            self.do_dump(silent=True)
        else:
            self.log(out.strip(), "err")
            if os.path.exists("out.bin"):
                try:
                    os.remove("out.bin")
                except Exception:
                    pass

    def build_pcmap(self, lsttxt):
        self.pcmap = {}
        for i, ln in enumerate(lsttxt.splitlines()):
            m = re.match(r"([0-9A-Fa-f]{8})\s+[0-9A-Fa-f]{8}", ln)
            if m:
                self.pcmap[int(m.group(1), 16)] = i

    def preload_trace(self):
        self.trace, self.halt = [], ""
        if not os.path.exists("out.bin"):
            return
        rc, out = sh("./emu -trace -steps 20000 out.bin")
        # emu returns 1 on step-limit for long loops; still parse what we got
        for ln in out.splitlines():
            if TRACE_RE.match(ln):
                self.trace.append(ln)
                if len(self.trace) >= 20000:
                    break
            elif ln.startswith("HALT"):
                self.halt = ln
        self.idx = 0
        self.set_regs(0, 0, 0, 65535, 0)
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
                self.curins.config(text=f"{mn} {op}")
                if pc in self.pcmap:
                    ln = self.pcmap[pc] + 1
                    self.lst.tag_add("cur", f"{ln}.0", f"{ln}.end")
                    self.lst.see(f"{ln}.0")
        elif self.halt:
            m = HALT_RE.match(self.halt)
            if m:
                self.set_regs(m.group(1), m.group(2), m.group(3), m.group(4), m.group(5))
                self.curins.config(text=self.halt)
                self.log(self.halt, "ok")

    def do_step(self):
        if not os.path.exists("out.bin"):
            self.log("assemble first", "err")
            return
        if not self.trace:
            self.preload_trace()
        if self.idx < len(self.trace):
            self.show_step()
            self.idx += 1
            if self.idx == len(self.trace) and self.halt:
                self.show_step()
        else:
            self.log("end", "err")

    def do_reset(self):
        self.idx = 0
        self.set_regs(0, 0, 0, 65535, 0)
        self.curins.config(text="-")
        self.lst.tag_remove("cur", "1.0", "end")
        self.log("reset", "ok")

    def do_run(self):
        if not os.path.exists("out.bin"):
            self.log("assemble first", "err")
            return
        rc, out = sh("./emu out.bin")
        self.log(out.strip(), "ok" if rc == 0 else "err")
        m = HALT_RE.search(out)
        if m:
            self.set_regs(m.group(1), m.group(2), m.group(3), m.group(4), m.group(5))
            self.curins.config(text=m.group(0))
        self.do_dump(silent=True)

    def do_dump(self, silent=False):
        if not os.path.exists("out.bin"):
            if not silent:
                self.log("assemble first", "err")
            return
        rc, out = sh("./emu -dump /tmp/iitps_dump.txt out.bin")
        try:
            lines = open("/tmp/iitps_dump.txt").read().splitlines()[:64]
            self.mem.delete("1.0", "end")
            self.mem.insert("1.0", "\n".join(lines))
        except Exception as e:
            if not silent:
                self.log(str(e), "err")

if __name__ == "__main__":
    r = tk.Tk()
    Studio(r)
    r.mainloop()
