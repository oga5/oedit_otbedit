
;; car - cddddr
(define (caar x) (car (car x)))
(define (cadr x) (car (cdr x)))
(define (cdar x) (cdr (car x)))
(define (cddr x) (cdr (cdr x)))
(define (caaar x) (car (car (car x))))
(define (caadr x) (car (car (cdr x))))
(define (cadar x) (car (cdr (car x))))
(define (caddr x) (car (cdr (cdr x))))
(define (cdaar x) (cdr (car (car x))))
(define (cdadr x) (cdr (car (cdr x))))
(define (cddar x) (cdr (cdr (car x))))
(define (cdddr x) (cdr (cdr (cdr x))))
(define (caaaar x) (car (car (car (car x)))))
(define (caaadr x) (car (car (car (cdr x)))))
(define (caadar x) (car (car (cdr (car x)))))
(define (caaddr x) (car (car (cdr (cdr x)))))
(define (cadaar x) (car (cdr (car (car x)))))
(define (cadadr x) (car (cdr (car (cdr x)))))
(define (caddar x) (car (cdr (cdr (car x)))))
(define (cadddr x) (car (cdr (cdr (cdr x)))))
(define (cdaaar x) (cdr (car (car (car x)))))
(define (cdaadr x) (cdr (car (car (cdr x)))))
(define (cdadar x) (cdr (car (cdr (car x)))))
(define (cdaddr x) (cdr (car (cdr (cdr x)))))
(define (cddaar x) (cdr (cdr (car (car x)))))
(define (cddadr x) (cdr (cdr (car (cdr x)))))
(define (cdddar x) (cdr (cdr (cdr (car x)))))
(define (cddddr x) (cdr (cdr (cdr (cdr x)))))

;; min-max
(define (_foldr fn x lst)
  (if (null? lst)
    x
    (_foldr fn (fn x (car lst)) (cdr lst))))
(define (min . lst)
  (_foldr (lambda (a b) (if (< a b) a b)) (car lst) (cdr lst)))
(define (max . lst)
  (_foldr (lambda (a b) (if (> a b) a b)) (car lst) (cdr lst)))

;; call/cc, delay, force
(define call/cc call-with-current-continuation)
(define-macro (delay x) `(lambda () ,x))
(define (force x) (if (closure? x) (x) x))

;; list
(define (list . x) x)
(define (list-tail x k)
  (if (zero? k) x (list-tail (cdr x) (- k 1))))
(define (list-ref x k) (car (list-tail x k)))
(define (list->vector x) (apply vector x))

(define (vector . x) `#(,@x))
(define (vector->list v)
  (let loop ((i (- (vector-length v) 1)) (l '()))
    (if (= i -1) 
      l
      (loop (- i 1) (cons (vector-ref v i) l)))))

;; map
(define (map proc . lists)
  (define (simple-map proc list)
    (if (pair? list)
        (cons (proc (car list)) (simple-map proc (cdr list)))
        '()))
  (if (pair? (car lists))
      (cons (apply proc (simple-map car lists))
            (apply map `(,proc ,@(simple-map cdr lists))))
      '()))

;; for-each
(define (for-each proc . lists)
  (if (pair? (car lists))
      (begin (apply proc (map car lists))
             (apply for-each `(,proc ,@(map cdr lists))))
      '()))

;; memq, memv, member
(define (_member-proc cmp obj lst)
  (cond
    ((null? lst) #f)
    ((cmp obj (car lst)) lst)
    (else (_member-proc cmp obj (cdr lst)))))
(define (memq obj alst) (_member-proc eq? obj alst))
(define (memv obj alst) (_member-proc eqv? obj alst))
(define (member obj alst) (_member-proc equal? obj alst))

;; assq, assv, assoc
(define (_association-list-proc cmp obj alst)
  (cond
    ((null? alst) #f)
    ((cmp obj (caar alst)) (car alst))
    (else (_association-list-proc cmp obj (cdr alst)))))
(define (assq obj alst) (_association-list-proc eq? obj alst))
(define (assv obj alst) (_association-list-proc eqv? obj alst))
(define (assoc obj alst) (_association-list-proc equal? obj alst))

;; do
(define-macro (do vars endtest . body)
  (let ((do-loop (gensym)))
    `(letrec
       ((,do-loop
          (lambda ,(map (lambda (x) (if (pair? x) (car x) x)) `,vars)
            (if ,(car endtest)
              (begin ,@(cdr endtest))
              (begin ,@body
                (,do-loop
                  ,@(map (lambda (x) (cond ((not (pair? x)) x)
                                           ((< (length x) 3) (car x))
                                           (else (caddr x)))) `,vars)))))))
       (,do-loop ,@(map (lambda (x) (if (and (pair? x) (pair? (cdr x)))
                                       (cadr x)
                                       #f)) `,vars)))))

;; case
(define-macro (case key . clauses)
  (define key-val (gensym))
  `(begin
    (define ,key-val ,key)
    (cond
      ,@(map
        (lambda (clause)
          (if (eq? (car clause) 'else)
            clause
            `((memv ,key-val ',(car clause)) ,@(cdr clause))))
        clauses))))

;; number
(define (abs x) (if (< x 0) (- x) x))
(define (zero? x) (= x 0))
(define (positive? x) (> x 0))
(define (negative? x) (< x 0))
(define (odd? x) (= (abs (remainder x 2)) 1))
(define (even? x) (= (remainder x 2) 0))

;; char
(define (char-numeric? c) (and (char<=? #\0 c) (char<=? c #\9)))
(define (char-upper-case? c) (and (char<=? #\A c) (char<=? c #\Z)))
(define (char-lower-case? c) (and (char<=? #\a c) (char<=? c #\z)))
(define (char-alphabetic? c) (or (char-upper-case? c) (char-lower-case? c)))
(define (char-whitespace? c) (or (char=? #\space c) (char=? #\newline c)
                                 (char=? #\return c) (char=? #\tab c)))
(define (_char_updown c op)
  (integer->char (op (char->integer c)
                     (- (char->integer #\a) (char->integer #\A)))))
(define (char-upcase c) (if (char-lower-case? c) (_char_updown c -) c))
(define (char-downcase c) (if (char-upper-case? c) (_char_updown c +) c))
(define (_char-cmp? fn c1 c2) (fn (char->integer c1) (char->integer c2)))
(define (_char-cmp-ci? fn c1 c2) (fn (char-upcase c1) (char-upcase c2)))
(define (char=? c1 c2) (_char-cmp? = c1 c2))
(define (char<? c1 c2) (_char-cmp? < c1 c2))
(define (char>? c1 c2) (_char-cmp? > c1 c2))
(define (char<=? c1 c2) (_char-cmp? <= c1 c2))
(define (char>=? c1 c2) (_char-cmp? >= c1 c2))
(define (char-ci=? c1 c2) (_char-cmp-ci? char=? c1 c2))
(define (char-ci<? c1 c2) (_char-cmp-ci? char<? c1 c2))
(define (char-ci>? c1 c2) (_char-cmp-ci? char>? c1 c2))
(define (char-ci<=? c1 c2) (_char-cmp-ci? char<=? c1 c2))
(define (char-ci>=? c1 c2) (_char-cmp-ci? char>=? c1 c2))

;; string
(define (list->string x)
  (let* ((len (length x)) (s (make-string len)))
    (let loop ((i 0) (lst x))
      (if (= i len)
        s
        (begin (string-set! s i (car lst))
          (loop (+ i 1) (cdr lst)))))))
(define (string . x) (list->string x))
(define (string->list s)
  (let loop ((i (- (string-length s) 1)) (l '()))
    (if (= i -1) 
      l
      (loop (- i 1) (cons (string-ref s i) l)))))
(define (string=? s1 s2) (_string-cmp? = s1 s2))
(define (string<? s1 s2) (_string-cmp? < s1 s2))
(define (string>? s1 s2) (_string-cmp? > s1 s2))
(define (string<=? s1 s2) (_string-cmp? <= s1 s2))
(define (string>=? s1 s2) (_string-cmp? >= s1 s2))
(define (string-ci=? s1 s2) (_string-cmp-ci? = s1 s2))
(define (string-ci<? s1 s2) (_string-cmp-ci? < s1 s2))
(define (string-ci>? s1 s2) (_string-cmp-ci? > s1 s2))
(define (string-ci<=? s1 s2) (_string-cmp-ci? <= s1 s2))
(define (string-ci>=? s1 s2) (_string-cmp-ci? >= s1 s2))

(define (string-copy s) (string-append s))

;; port
(define (call-with-input-file fname proc)
  (let ((port (open-input-file fname)))
    (define result (apply proc port '()))
    (close-input-port port)
    result))
(define (call-with-output-file fname proc)
  (let ((port (open-output-file fname)))
    (define result (apply proc port '()))
    (close-output-port port)
    result))

;; receive (srfi-8)
(define-macro (receive formals expression . body)
  `(call-with-values (lambda () ,expression)
                     (lambda ,formals ,@body)))

;; cut, cute (srfi-26)
(define-macro (cut . exprs)
  (define (mk_form l s b)
    (if (null? l)
      `(lambda ,(reverse s) ,(reverse b))
      (case (car l)
        ((<>) (let ((v (gensym)))
          (mk_form (cdr l) (cons v s) (cons v b))))
        ((<...>)
          (if (pair? (cdr l))
            (error "<...> not on end of expr"))
          (let ((v (gensym)))
            `(lambda (,@(reverse s) . ,v) (apply ,@(reverse (cons v b))))))
        (else (mk_form (cdr l) s (cons (car l) b))))))
  (mk_form exprs '() '()))

(define-macro (cute . exprs)
  (define (mk_let l e b)
    (if (null? l)
      `(let ,(reverse e) (cut ,@(reverse b)))
      (case (car l)
        ((<> <...>) (mk_let (cdr l) e (cons (car l) b)))
        (else 
          (let ((v (gensym)))
            (mk_let (cdr l)
                    (cons `(,v ,(car l)) e)
                    (cons v b)))))))
  (mk_let exprs '() '()))

;; :optional, check-arg (for srfi-1)
(define (:optional a b) (if (null? a) b (car a)))
(define (check-arg pred val caller)
  (let lp ((val val))
    (if (pred val) val (lp (error "bad argument" val pred caller)))))

;; object system
(define-macro (define-class name supers slots)
  `(define ,name (_define-class (symbol->string ',name) ',supers ',slots)))

;; misc.
(define quotient /)
