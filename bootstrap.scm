;; -*- mode: Lisp; -*-

(define caar (lambda (x) (car (car x))))
(define cdar (lambda (x) (cdr (car x))))
(define cadr (lambda (x) (car (cdr x))))
(define cddr (lambda (x) (cdr (cdr x))))
