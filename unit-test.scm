;; -*- mode: Scheme; -*-

(print 'Welcome!)

;; prints expression if it doesn't evaluate to #t
(define assert
  (lambda (x)
    (if (not (eq? (eval x) #t))
        (print 'Test x 'Failed!))))

;; define
(define ten 10)
(define plus +)
(define true #t)
(assert '(eq? 10 ten))
(assert '(eq? + plus))
(assert '(eq? true #t))

;; if
(assert '(eq? 10 (if #t 10 11)))
(assert '(eq? 11 (if #f 10 11)))

;; quote
(assert '(eq? 10 (quote 10)))
(assert '(eq? #t (quote #t)))

;; set
(define xxx 10)
(set! xxx 20)
(assert '(eq? xxx 20))

;; begin
(define xxx (begin
              (define yyy 20)
              (define zzz 30)
              10))
(assert '(eq? xxx 10))
(assert '(eq? yyy 20))
(assert '(eq? zzz 30))

;; lambda
(define plus-ten (lambda (x) (+ x ten)))
(assert '(eq? (plus-ten 10) 20))

;; boolean?
(assert '(boolean? #f))
(assert '(not (boolean? ())))
(assert '(not (boolean? 'foo)))
(assert '(not (boolean? 0.0)))
(assert '(not (boolean? '(1 2))))
(assert '(not (boolean? (make-environment))))
(assert '(not (boolean? if)))
(assert '(not (boolean? boolean?)))

;; null?
(assert '(not (null? #f)))
(assert '(null? ()))
(assert '(not (null? 'foo)))
(assert '(not (null? 0.0)))
(assert '(not (null? '(1 2))))
(assert '(not (null? (make-environment))))
(assert '(not (null? if)))
(assert '(not (null? null?)))

;; symbol?
(assert '(not (symbol? #f)))
(assert '(not (symbol? ())))
(assert '(symbol? 'foo))
(assert '(not (symbol? 0.0)))
(assert '(not (symbol? '(1 2))))
(assert '(not (symbol? (make-environment))))
(assert '(not (symbol? if)))
(assert '(not (symbol? symbol?)))

;; number?
(assert '(not (number? #f)))
(assert '(not (number? ())))
(assert '(not (number? 'foo)))
(assert '(number? 0.0))
(assert '(not (number? '(1 2))))
(assert '(not (number? (make-environment))))
(assert '(not (number? if)))
(assert '(not (number? number?)))

;; pair?
(assert '(not (pair? #f)))
(assert '(not (pair? ())))
(assert '(not (pair? 'foo)))
(assert '(not (pair? 0.0)))
(assert '(pair? '(1 2)))
(assert '(not (pair? (make-environment))))
(assert '(not (pair? if)))
(assert '(not (pair? number?)))

;; environment?
(assert '(not (environment? #f)))
(assert '(not (environment? ())))
(assert '(not (environment? 'foo)))
(assert '(not (environment? 0.0)))
(assert '(not (environment? '(1 2))))
(assert '(environment? (make-environment)))
(assert '(not (environment? if)))
(assert '(not (environment? number?)))

;; procedure?
(assert '(not (procedure? #f)))
(assert '(not (procedure? ())))
(assert '(not (procedure? 'foo)))
(assert '(not (procedure? 0.0)))
(assert '(not (procedure? '(1 2))))
(assert '(not (procedure? (make-environment))))
(assert '(not (procedure? if)))
(assert '(procedure? number?))

;; eq?
(assert '(eq? #t #t))
(assert '(eq? 10 10))
(assert '(eq? eq? eq?))
(assert '(eq? if if))
(assert '(eq? () ()))
(assert '(not (eq? '(1) '(1))))

;; equal?
(assert '(equal? #t #t))
(assert '(equal? 10 10))
(assert '(equal? eq? eq?))
(assert '(equal? if if))
(assert '(equal? () ()))
(assert '(equal? '(1) '(1)))
(assert '(equal? '(1 if #t (key . value) ()) '(1 if #t (key . value) ())))

;; cons
(assert '(equal? '(1 2 3 . 4) (cons 1 (cons 2 (cons 3 4)))))

;; car
(assert '(eq? 1 (car '(1 2 3 . 4))))

;; cdr
(assert '(equal? '(2 3 . 4) (cdr '(1 2 3 . 4))))

;; set-car!
(define temp '(1 2 3 . 4))
(set-car! temp 4)
(assert '(equal? temp '(4 2 3 . 4)))

;; set-cdr!
(set-cdr! temp 10)
(assert '(equal? temp '(4 . 10)))

;; +
(assert '(eq? (+) 0))
(assert '(eq? (+ 1) 1))
(assert '(eq? (+ 1 2 3) 6))

;; -
(assert '(eq? (-) 0))
(assert '(eq? (- 1) -1))
(assert '(eq? (- 1 2 3) -4))

;; *
(assert '(eq? (*) 1))
(assert '(eq? (* 1) 1))
(assert '(eq? (* 1 2 3) 6))

;; /
(assert '(eq? (/ 1 2) 0.5))
(assert '(eq? (/ 1 2 2) 0.25))

;; math comp
(assert '(> (+ 1 1) 1))
(assert '(>= (+ 1 1) 1))
(assert '(>= 2 (+ 1 1)))
(assert '(<= 1 (+ 1 1)))
(assert '(<= 2 (+ 1 1)))
(assert '(= (+ 1 1) 2))

;; math functions
(assert '(eq? (abs -2) 2))

;; not
(assert (not #f))
(assert (not ()))
(assert (not (not #t)))

;; make-environment
(assert '(environment? (make-environment)))

;; eval
(assert '(eq? 10 (eval 10)))
(assert '(eq? #t (eval #t)))
(assert '(eq? 10 (eval '(+ 5 5))))

