" ============================================================================ "
"   UNIFIED VIM CONFIGURATION (.vimrc)
"   Languages: C & Python
"   System: Vim
" ============================================================================ "

" 0. BASIC SETUP
set nocompatible
filetype off

" 1. VIM-PLUG BOOTSTRAP (Required to auto-install your plugin manager)
let s:plug_path = expand('~/.vim/autoload/plug.vim')

" 2. PYTHON SYNTAX CONFIGURATION (Required for f-strings)
" This MUST sit before 'syntax on' or 'plug#begin' to guarantee it works.
let g:python_highlight_all = 1
if !filereadable(s:plug_path)
  echo "Installing vim-plug..."
  if executable('curl')
    silent !curl -fLo ~/.vim/autoload/plug.vim --create-dirs https://raw.githubusercontent.com/junegunn/vim-plug/master/plug.vim
  elseif executable('wget')
    call mkdir(expand('~/.vim/autoload'), 'p')
    silent !wget -O ~/.vim/autoload/plug.vim https://raw.githubusercontent.com/junegunn/vim-plug/master/plug.vim
  endif
  if filereadable(s:plug_path)
    source ~/.vim/autoload/plug.vim
    autocmd VimEnter * PlugInstall --sync | source $MYVIMRC
  endif
endif

" 2. PLUGINS
if filereadable(s:plug_path)
    call plug#begin('~/.vim/plugged')
        Plug 'morhetz/gruvbox'       " Theme
        Plug 'vim-python/python-syntax' " <--- ADDED: Better Python Syntax (f-strings)
"        Plug 'dense-analysis/ale'    " Linting (Works for both C and Python)
        Plug 'preservim/nerdtree'    " File Explorer
    call plug#end()
endif

" 3. GLOBAL LOOK AND FEEL
set background=dark
filetype plugin indent on

if has("termguicolors")
    set termguicolors
endif

try
    colorscheme gruvbox
catch
    colorscheme default
endtry

syntax enable
let &t_SI = "\<Esc>[6 q"
let &t_EI = "\<Esc>[2 q"

set number
set relativenumber

" 4. LANGUAGE SPECIFIC CONFIGURATIONS (The Magic Part)

" --- C / C++ Configuration ---
" Triggers only for .c, .cpp, .h files
augroup C_Config
    autocmd!
    " Use 2 spaces, C-style indentation
    autocmd FileType c,cpp,objc setlocal tabstop=4 shiftwidth=4 expandtab cindent
    " Map F5 to Compile & Run (GCC)
    autocmd FileType c,cpp,objc nnoremap <buffer> <F5> :w <bar> !gcc % -o %< && ./%<<CR>
augroup END

" --- Python Configuration ---
augroup Python_Config
    autocmd!
    " Standard settings
    autocmd FileType python setlocal tabstop=4 shiftwidth=4 expandtab autoindent smartindent
    autocmd FileType python nnoremap <buffer> <F5> :w <bar> !python3 %<CR>
    
    " --- CUSTOM COLORS ---
    " Force 'self' keyword to be Cyan
    autocmd FileType python highlight pythonSelf  ctermfg=Cyan guifg=#00FFFF
augroup END

" 5. ALE (LINTER) SETTINGS
" Define tools for both languages here
let g:ale_linters = {
\   'c': ['gcc', 'clang'],
\   'python': ['flake8', 'pylint'],
\}

let g:ale_fixers = {
\   'c': ['clang-format'],
\   'python': ['black'],
\}

let g:ale_fix_on_save = 0

" Navigation
nmap <silent> <C-k> <Plug>(ale_previous_wrap)
nmap <silent> <C-j> <Plug>(ale_next_wrap)

" 6. GLOBAL KEY MAPPINGS
let mapleader = " "
nnoremap <leader>pv :NERDTreeToggle<CR>
nnoremap <leader>w :w<CR>
