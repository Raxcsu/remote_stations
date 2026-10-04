"""
Inicialización del proyecto: crea la base de datos y registra los nodos.
Ejecutado automáticamente por setup.bat.
"""

from db import get_db, init_schema, registrar_dispositivos


def init():
    conn = get_db()
    init_schema(conn)
    print("\nRegistrando nodos:")
    registrar_dispositivos(conn)
    conn.close()


if __name__ == '__main__':
    init()
    print("\nBase de datos lista en data/temperatura.db")
