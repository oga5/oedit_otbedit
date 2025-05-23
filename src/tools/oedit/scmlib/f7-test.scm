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
    