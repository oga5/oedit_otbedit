;;
;; 起動時に実行するスクリプトを記述してください
;;

;; ==================================================================	  
;; マクロの実行中止キーを設定
;; ==================================================================	  
(app-set-cancel-key "Ctrl+\\")

;; ==================================================================	  
;; ショートカットキーを設定
;; ==================================================================
;; HTML変換
(app-set-key "F6"
  (lambda ()
    (define cnt
	  (apply + (map editor-replace-selected-string
                 '("&" "<" ">" "\"")
                 '("&amp;" "&lt;" "&gt;" "&quot;"))))
    (app-status-bar-msg cnt "件置換しました(HTML変換)"))
  "HTML変換")

(define (regexp-replace-list str l1 l2)
  (let loop ((s str) (lst1 l1) (lst2 l2))
    (if (null? lst1)
      s
      (begin 
       (set! s (regexp-replace-all (car lst1) s (car lst2)))
       (loop s (cdr lst1) (cdr lst2))))))

(app-set-key "F7"
  (lambda () (editor-paste-string (regexp-replace-list (win-get-clipboard-text)
    '(#/&/ #/</ #/>/ #/\"/)
    '("&amp;" "&lt;" "&gt;" "&quot;")))))
    
;; ==================================================================
;; ファイルを開いたとき、カーソル位置を復元する
;; ==================================================================
(let ((cursor-save-file-cnt 10)
      (cursor-save-file-name 
	    (string-append (app-get-tool-dir) "oedit_cur_pos.txt")))

  ;; ファイルからファイル名とカーソル位置の対応を取得
  (define (load-pos-data)
    (if (file-exists? cursor-save-file-name)
      (let loop ((r (open-input-file cursor-save-file-name)) (l '()))
        (if (eof-object? (peek-char r))
          (begin (close-input-port r) (reverse l))
          (loop r (cons (read r) l))))
      '()))

  ;; カーソル位置を復元
  (define (restore-cursor-pos)
    (let ((fname (editor-get-filename)))
      (if (and (string? fname) (> (string-length fname) 0))
        (let loop ((lst (load-pos-data)))
          (if (null? lst)
            #f
            (if (string=? (caar lst) fname)
              (editor-set-row-col (cadar lst) (caddar lst))
              (loop (cdr lst))))))))

  ;; 現在のカーソル位置を保存
  (define (save-cursor-pos)
    (let ((fname (editor-get-filename)))
      (if (and (string? fname) (> (string-length fname) 0))
        (let ((lst (load-pos-data))
              (w (open-output-file cursor-save-file-name))
              (record (list fname (editor-get-cur-row) (editor-get-cur-col))))
          (write record w)
          (let loop ((l lst) (i 0))
            (if (or (null? l) (= i cursor-save-file-cnt))
              (close-output-port w)
              (begin
                (if (not (string=? (caar l) fname))
                  (write (car l) w))
                (loop (cdr l) (+ i 1)))))))))

  ;; イベントハンドラを定義
  (app-set-event-handler "on-open-file" restore-cursor-pos)
  (app-set-event-handler "on-close-file" save-cursor-pos))

;; ==================================================================
;; 選択範囲のバイト数・文字数を表示する
;; ==================================================================
(app-set-event-handler "on-selection-changed" 
  (lambda () 
    (let ((bytes (editor-get-selected-string-bytes)) 
          (chars (editor-get-selected-string-chars)))
      (if (= bytes 0)
        (app-status-bar-msg "")
        (app-status-bar-msg bytes "bytes," chars "chars")))))

;; ==================================================================
;; Alt+矢印キーで矩形選択
;; ==================================================================
;(app-set-key "Alt+LEFT" 
;  (lambda () (editor-set-box-select) (editor-backward-char 1 #t)))
;(app-set-key "Alt+RIGHT" 
;  (lambda () (editor-set-box-select) (editor-forward-char 1 #t)))
;(app-set-key "Alt+UP" 
;  (lambda () (editor-set-box-select) (editor-previous-line 1 #t)))
;(app-set-key "Alt+DOWN" 
;  (lambda () (editor-set-box-select) (editor-next-line 1 #t)))
(app-set-key "F2" 
  (lambda () (display (editor-box-select?))))

;; ==================================================================
;; 時間のかかる処理のテスト
;; ==================================================================
(app-set-key "Alt+F11"
  (lambda ()
    (begin
      (define tak (lambda (x y z)
        (cond
          ((> x y) (tak (tak (- x 1) y z)
                   (tak (- y 1) z x)
                   (tak (- z 1) x y))) (#t y))))
      (display (tak 12 6 0)))))

;; ==================================================================
;; cell slotを一杯にするテスト
;; ==================================================================
(app-set-key "Ctrl+F11"
  (lambda ()
    (define (mugenloop)
      (let loop ((l '()) (cnt 0))
        (begin (app-status-bar-msg cnt) (loop (cons 1 l) (+ cnt 1)))))
    (mugenloop)))

;; ==================================================================
;; Input Boxに入力されたS式を評価する
;; ==================================================================
(app-set-key "Ctrl+0"
  (lambda ()
    (let ((s (app-input-box "S式を入力")))
      (if (string? s)
        (display (eval (read (open-input-string s))))))))

;(app-set-event-handler "on-activate-app" 
;  (lambda () 
;    (app-msg-box "activate app:" (editor-get-filename))))
;(app-set-event-handler "on-activate-app" 
;  (lambda () 
;    (app-status-bar-msg "activate app:" (editor-get-filename))))

