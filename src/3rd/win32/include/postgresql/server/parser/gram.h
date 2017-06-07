/* A Bison parser, made by GNU Bison 2.5.  */

/* Bison interface for Yacc-like parsers in C
   
      Copyright (C) 1984, 1989-1990, 2000-2011 Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     IDENT = 258,
     FCONST = 259,
     SCONST = 260,
     BCONST = 261,
     XCONST = 262,
     Op = 263,
     ICONST = 264,
     PARAM = 265,
     TYPECAST = 266,
     DOT_DOT = 267,
     COLON_EQUALS = 268,
     ABORT_P = 269,
     ABSOLUTE_P = 270,
     ACCESS = 271,
     ACTION = 272,
     ADD_P = 273,
     ADMIN = 274,
     AFTER = 275,
     AGGREGATE = 276,
     ALL = 277,
     ALSO = 278,
     ALTER = 279,
     ALWAYS = 280,
     ANALYSE = 281,
     ANALYZE = 282,
     AND = 283,
     ANY = 284,
     ARRAY = 285,
     AS = 286,
     ASC = 287,
     ASSERTION = 288,
     ASSIGNMENT = 289,
     ASYMMETRIC = 290,
     AT = 291,
     AUTHORIZATION = 292,
     BACKWARD = 293,
     BEFORE = 294,
     BEGIN_P = 295,
     BETWEEN = 296,
     BIGINT = 297,
     BINARY = 298,
     BIT = 299,
     BOOLEAN_P = 300,
     BOTH = 301,
     BY = 302,
     CACHE = 303,
     CALLED = 304,
     CASCADE = 305,
     CASCADED = 306,
     CASE = 307,
     CAST = 308,
     CATALOG_P = 309,
     CHAIN = 310,
     CHAR_P = 311,
     CHARACTER = 312,
     CHARACTERISTICS = 313,
     CHECK = 314,
     CHECKPOINT = 315,
     CLASS = 316,
     CLOSE = 317,
     CLUSTER = 318,
     COALESCE = 319,
     COLLATE = 320,
     COLUMN = 321,
     COMMENT = 322,
     COMMENTS = 323,
     COMMIT = 324,
     COMMITTED = 325,
     CONCURRENTLY = 326,
     CONFIGURATION = 327,
     CONNECTION = 328,
     CONSTRAINT = 329,
     CONSTRAINTS = 330,
     CONTENT_P = 331,
     CONTINUE_P = 332,
     CONVERSION_P = 333,
     COPY = 334,
     COST = 335,
     CREATE = 336,
     CREATEDB = 337,
     CREATEROLE = 338,
     CREATEUSER = 339,
     CROSS = 340,
     CSV = 341,
     CURRENT_P = 342,
     CURRENT_CATALOG = 343,
     CURRENT_DATE = 344,
     CURRENT_ROLE = 345,
     CURRENT_SCHEMA = 346,
     CURRENT_TIME = 347,
     CURRENT_TIMESTAMP = 348,
     CURRENT_USER = 349,
     CURSOR = 350,
     CYCLE = 351,
     DATA_P = 352,
     DATABASE = 353,
     DAY_P = 354,
     DEALLOCATE = 355,
     DEC = 356,
     DECIMAL_P = 357,
     DECLARE = 358,
     DEFAULT = 359,
     DEFAULTS = 360,
     DEFERRABLE = 361,
     DEFERRED = 362,
     DEFINER = 363,
     DELETE_P = 364,
     DELIMITER = 365,
     DELIMITERS = 366,
     DESC = 367,
     DICTIONARY = 368,
     DISABLE_P = 369,
     DISCARD = 370,
     DISTINCT = 371,
     DO = 372,
     DOCUMENT_P = 373,
     DOMAIN_P = 374,
     DOUBLE_P = 375,
     DROP = 376,
     EACH = 377,
     ELSE = 378,
     ENABLE_P = 379,
     ENCODING = 380,
     ENCRYPTED = 381,
     END_P = 382,
     ENUM_P = 383,
     ESCAPE = 384,
     EXCEPT = 385,
     EXCLUDE = 386,
     EXCLUDING = 387,
     EXCLUSIVE = 388,
     EXECUTE = 389,
     EXISTS = 390,
     EXPLAIN = 391,
     EXTERNAL = 392,
     EXTRACT = 393,
     FALSE_P = 394,
     FAMILY = 395,
     FETCH = 396,
     FIRST_P = 397,
     FLOAT_P = 398,
     FOLLOWING = 399,
     FOR = 400,
     FORCE = 401,
     FOREIGN = 402,
     FORWARD = 403,
     FREEZE = 404,
     FROM = 405,
     FULL = 406,
     FUNCTION = 407,
     FUNCTIONS = 408,
     GLOBAL = 409,
     GRANT = 410,
     GRANTED = 411,
     GREATEST = 412,
     GROUP_P = 413,
     HANDLER = 414,
     HAVING = 415,
     HEADER_P = 416,
     HOLD = 417,
     HOUR_P = 418,
     IDENTITY_P = 419,
     IF_P = 420,
     ILIKE = 421,
     IMMEDIATE = 422,
     IMMUTABLE = 423,
     IMPLICIT_P = 424,
     IN_P = 425,
     INCLUDING = 426,
     INCREMENT = 427,
     INDEX = 428,
     INDEXES = 429,
     INHERIT = 430,
     INHERITS = 431,
     INITIALLY = 432,
     INLINE_P = 433,
     INNER_P = 434,
     INOUT = 435,
     INPUT_P = 436,
     INSENSITIVE = 437,
     INSERT = 438,
     INSTEAD = 439,
     INT_P = 440,
     INTEGER = 441,
     INTERSECT = 442,
     INTERVAL = 443,
     INTO = 444,
     INVOKER = 445,
     IS = 446,
     ISNULL = 447,
     ISOLATION = 448,
     JOIN = 449,
     KEY = 450,
     LANGUAGE = 451,
     LARGE_P = 452,
     LAST_P = 453,
     LC_COLLATE_P = 454,
     LC_CTYPE_P = 455,
     LEADING = 456,
     LEAST = 457,
     LEFT = 458,
     LEVEL = 459,
     LIKE = 460,
     LIMIT = 461,
     LISTEN = 462,
     LOAD = 463,
     LOCAL = 464,
     LOCALTIME = 465,
     LOCALTIMESTAMP = 466,
     LOCATION = 467,
     LOCK_P = 468,
     LOGIN_P = 469,
     MAPPING = 470,
     MATCH = 471,
     MAXVALUE = 472,
     MINUTE_P = 473,
     MINVALUE = 474,
     MODE = 475,
     MONTH_P = 476,
     MOVE = 477,
     NAME_P = 478,
     NAMES = 479,
     NATIONAL = 480,
     NATURAL = 481,
     NCHAR = 482,
     NEXT = 483,
     NO = 484,
     NOCREATEDB = 485,
     NOCREATEROLE = 486,
     NOCREATEUSER = 487,
     NOINHERIT = 488,
     NOLOGIN_P = 489,
     NONE = 490,
     NOSUPERUSER = 491,
     NOT = 492,
     NOTHING = 493,
     NOTIFY = 494,
     NOTNULL = 495,
     NOWAIT = 496,
     NULL_P = 497,
     NULLIF = 498,
     NULLS_P = 499,
     NUMERIC = 500,
     OBJECT_P = 501,
     OF = 502,
     OFF = 503,
     OFFSET = 504,
     OIDS = 505,
     ON = 506,
     ONLY = 507,
     OPERATOR = 508,
     OPTION = 509,
     OPTIONS = 510,
     OR = 511,
     ORDER = 512,
     OUT_P = 513,
     OUTER_P = 514,
     OVER = 515,
     OVERLAPS = 516,
     OVERLAY = 517,
     OWNED = 518,
     OWNER = 519,
     PARSER = 520,
     PARTIAL = 521,
     PARTITION = 522,
     PASSWORD = 523,
     PLACING = 524,
     PLANS = 525,
     POSITION = 526,
     PRECEDING = 527,
     PRECISION = 528,
     PRESERVE = 529,
     PREPARE = 530,
     PREPARED = 531,
     PRIMARY = 532,
     PRIOR = 533,
     PRIVILEGES = 534,
     PROCEDURAL = 535,
     PROCEDURE = 536,
     QUOTE = 537,
     RANGE = 538,
     READ = 539,
     REAL = 540,
     REASSIGN = 541,
     RECHECK = 542,
     RECURSIVE = 543,
     REFERENCES = 544,
     REINDEX = 545,
     RELATIVE_P = 546,
     RELEASE = 547,
     RENAME = 548,
     REPEATABLE = 549,
     REPLACE = 550,
     REPLICA = 551,
     RESET = 552,
     RESTART = 553,
     RESTRICT = 554,
     RETURNING = 555,
     RETURNS = 556,
     REVOKE = 557,
     RIGHT = 558,
     ROLE = 559,
     ROLLBACK = 560,
     ROW = 561,
     ROWS = 562,
     RULE = 563,
     SAVEPOINT = 564,
     SCHEMA = 565,
     SCROLL = 566,
     SEARCH = 567,
     SECOND_P = 568,
     SECURITY = 569,
     SELECT = 570,
     SEQUENCE = 571,
     SEQUENCES = 572,
     SERIALIZABLE = 573,
     SERVER = 574,
     SESSION = 575,
     SESSION_USER = 576,
     SET = 577,
     SETOF = 578,
     SHARE = 579,
     SHOW = 580,
     SIMILAR = 581,
     SIMPLE = 582,
     SMALLINT = 583,
     SOME = 584,
     STABLE = 585,
     STANDALONE_P = 586,
     START = 587,
     STATEMENT = 588,
     STATISTICS = 589,
     STDIN = 590,
     STDOUT = 591,
     STORAGE = 592,
     STRICT_P = 593,
     STRIP_P = 594,
     SUBSTRING = 595,
     SUPERUSER_P = 596,
     SYMMETRIC = 597,
     SYSID = 598,
     SYSTEM_P = 599,
     TABLE = 600,
     TABLES = 601,
     TABLESPACE = 602,
     TEMP = 603,
     TEMPLATE = 604,
     TEMPORARY = 605,
     TEXT_P = 606,
     THEN = 607,
     TIME = 608,
     TIMESTAMP = 609,
     TO = 610,
     TRAILING = 611,
     TRANSACTION = 612,
     TREAT = 613,
     TRIGGER = 614,
     TRIM = 615,
     TRUE_P = 616,
     TRUNCATE = 617,
     TRUSTED = 618,
     TYPE_P = 619,
     UNBOUNDED = 620,
     UNCOMMITTED = 621,
     UNENCRYPTED = 622,
     UNION = 623,
     UNIQUE = 624,
     UNKNOWN = 625,
     UNLISTEN = 626,
     UNTIL = 627,
     UPDATE = 628,
     USER = 629,
     USING = 630,
     VACUUM = 631,
     VALID = 632,
     VALIDATOR = 633,
     VALUE_P = 634,
     VALUES = 635,
     VARCHAR = 636,
     VARIADIC = 637,
     VARYING = 638,
     VERBOSE = 639,
     VERSION_P = 640,
     VIEW = 641,
     VOLATILE = 642,
     WHEN = 643,
     WHERE = 644,
     WHITESPACE_P = 645,
     WINDOW = 646,
     WITH = 647,
     WITHOUT = 648,
     WORK = 649,
     WRAPPER = 650,
     WRITE = 651,
     XML_P = 652,
     XMLATTRIBUTES = 653,
     XMLCONCAT = 654,
     XMLELEMENT = 655,
     XMLFOREST = 656,
     XMLPARSE = 657,
     XMLPI = 658,
     XMLROOT = 659,
     XMLSERIALIZE = 660,
     YEAR_P = 661,
     YES_P = 662,
     ZONE = 663,
     NULLS_FIRST = 664,
     NULLS_LAST = 665,
     WITH_TIME = 666,
     POSTFIXOP = 667,
     UMINUS = 668
   };
#endif



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 2068 of yacc.c  */
#line 144 "gram.y"

	core_YYSTYPE		core_yystype;
	/* these fields must match core_YYSTYPE: */
	int					ival;
	char				*str;
	const char			*keyword;

	char				chr;
	bool				boolean;
	JoinType			jtype;
	DropBehavior		dbehavior;
	OnCommitAction		oncommit;
	List				*list;
	Node				*node;
	Value				*value;
	ObjectType			objtype;
	TypeName			*typnam;
	FunctionParameter   *fun_param;
	FunctionParameterMode fun_param_mode;
	FuncWithArgs		*funwithargs;
	DefElem				*defelt;
	SortBy				*sortby;
	WindowDef			*windef;
	JoinExpr			*jexpr;
	IndexElem			*ielem;
	Alias				*alias;
	RangeVar			*range;
	IntoClause			*into;
	WithClause			*with;
	A_Indices			*aind;
	ResTarget			*target;
	struct PrivTarget	*privtarget;
	AccessPriv			*accesspriv;
	InsertStmt			*istmt;
	VariableSetStmt		*vsetstmt;



/* Line 2068 of yacc.c  */
#line 502 "gram.h"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif



#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
} YYLTYPE;
# define yyltype YYLTYPE /* obsolescent; will be withdrawn */
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif



