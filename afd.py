import sys


def load_config(self, path: str):
with open(path, 'r', encoding='utf-8') as f:
for raw in f:
line = raw.strip()
if not line or line.startswith('#'):
continue
if ':' not in line:
raise ValueError(f"Línea inválida en conf: {line}")
key, value = [x.strip() for x in line.split(':', 1)]
if key == 'states':
self.states = [s.strip() for s in value.split(',') if s.strip()]
elif key == 'alphabet':
self.alphabet = [s.strip() for s in value.split(',') if s.strip()]
for sym in self.alphabet:
if len(sym) != 1:
raise ValueError(f"Símbolo no es de 1 carácter: {sym}")
elif key == 'start':
self.start = value
elif key == 'accept':
self.accept = [s.strip() for s in value.split(',') if s.strip()]
elif key == 'transition':
# formato: origen,simbolo->destino
left, dest = [x.strip() for x in value.split('->', 1)]
origin, symbol = [x.strip() for x in left.split(',', 1)]
if len(symbol) != 1:
raise ValueError(f"Símbolo no es de 1 carácter: {symbol}")
self.transitions[(origin, symbol)] = dest
else:
raise ValueError(f"Clave desconocida: {key}")
# Validaciones básicas
if not self.states or not self.alphabet or not self.start or not self.accept:
raise ValueError("Configuración incompleta (states, alphabet, start, accept)")
if self.start not in self.states:
raise ValueError("Estado inicial no está en states")
for a in self.accept:
if a not in self.states:
raise ValueError(f"Estado de aceptación desconocido: {a}")


def run(self, s: str) -> bool:
current = self.start
for ch in s:\
if ch not in self.alphabet:
return False
nxt = self.transitions.get((current, ch))
if nxt is None:
return False
current = nxt
return current in self.accept




def main():
if len(sys.argv) != 3:
print("Uso: python afd.py Conf.txt Cadenas.txt")
sys.exit(1)
conf_path, cadenas_path = sys.argv[1], sys.argv[2]
dfa = DFA()
dfa.load_config(conf_path)
with open(cadenas_path, 'r', encoding='utf-8') as f:
for raw in f:
line = raw.rstrip('\n')
stripped = line.strip()
if not stripped or stripped.startswith('#'):
continue
res = dfa.run(stripped)
print(f"{stripped}: {'ACCEPTED' if res else 'REJECTED'}")


if __name__ == '__main__':
main()