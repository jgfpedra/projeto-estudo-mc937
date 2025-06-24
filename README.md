# projeto-estudo-mc937

Simulação física e visualização 3D de modelos OBJ com colisão, rotação e interação, desenvolvido para a disciplina MC937 (Unicamp).

## Descrição

Este projeto simula a física de corpos rígidos (incluindo torus, corda e outros modelos) com colisão, rotação realista, vento e interação com o solo, exportando animações quadro a quadro em OBJ. O sistema utiliza OpenGL, GLFW, GLEW e GLM para renderização e matemática vetorial.

## Requisitos

- Linux
- [GLFW](https://www.glfw.org/)
- [GLEW](http://glew.sourceforge.net/)
- [GLM](https://github.com/g-truc/glm)
- CMake
- Compilador C++ (g++ recomendado)

## Compilação

```bash
mkdir build
cd build
cmake ..
make
```

## Execução

Execute o programa passando **três arquivos OBJ** como argumento (exemplo de torus):

```bash
./projeto-final ../models/torus_1.obj ../models/torus_2.obj ../models/torus_3.obj
```

- Os modelos serão carregados, simulados e renderizados.
- A cada frame, os modelos são exportados para a pasta `model_animations/` em formato OBJ.

## Controles

- **WASD**: movimentação da câmera
- **Mouse**: rotação da câmera (primeira pessoa)
- **ESC**: sair

## Estrutura do Projeto

```
src/
  core/         # Carregamento e exportação de OBJ, estrutura dos modelos
  graphics/     # Shaders, câmera, iluminação, desenho dos modelos
  physics/      # Simulação física, colisão, rotação, animação
  window/       # Criação da janela GLFW
main.cpp        # Loop principal, integração de tudo
```

## Exportação de Animação

- A cada frame, os modelos são exportados para `model_animations/anim_model*_frame_*.obj`
- Isso permite criar animações ou análises quadro a quadro.