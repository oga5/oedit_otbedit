
;; let-values
(define-macro (let-values vars . body)
  (define (gensym-list lst)
    (cond
      ((pair? lst) (cons (gensym) (gensym-list (cdr lst))))
      ((null? lst) '())
      (else (gensym))))

  (define (undot-list lst)
    (cond
      ((pair? lst) (cons (car lst) (undot-list (cdr lst))))
      ((null? lst) '())
      (else (list lst))))

  (if (null? vars)
      (car body)
      (let* ((var (car vars))
             (tmp-syms (gensym-list (car var)))
             (let-vars (map (lambda (s t) (list s t))
                            (undot-list (car var))
                            (undot-list tmp-syms))))
        `(call-with-values (lambda () ,(cadr var))
           (lambda ,tmp-syms
             (let-values ,(cdr vars) (let ,let-vars ,@body)))))))

;; let*-values
(define-macro (let*-values vars . body)
  (if (null? vars)
      (car body)
      (let ((var (car vars)))
        `(call-with-values (lambda () ,(cadr var))
           (lambda ,(car var) (let*-values ,(cdr vars) ,@body))))))

