CREATE EXTENSION pg_climb;
 
SELECT EXISTS (
    SELECT 1 FROM pg_type WHERE typname = 'grade'
);

-- Grades can be created by their well-known formats with `grade_in()`
-- (`::grade`) and outputted in those same formats with `grade_out()`
SELECT 'V5'::grade;
SELECT 'F7A'::grade;
SELECT '5.12a'::grade;

-- invalid because "nope" isn't a grade type
CREATE TABLE grades_1(grade grade(nope));

-- any type of grade can be inserted into a column with no typmod
CREATE TABLE grades_2(grade grade);
INSERT INTO grades_2 VALUES ('V5'::grade), ('F7A+'::grade);

-- only values of the given typmod can be inserted when typmod is specified
CREATE TABLE grades_3(grade grade(verm));
INSERT INTO grades_3 VALUES ('V5'::grade);
INSERT INTO grades_3 VALUES ('F7A+'::grade);

-- b-tree operators allow for constraints such as UNIQUE to be used
CREATE TABLE grades_unique(grade grade, UNIQUE(grade));
INSERT INTO grades_unique VALUES
    ('V2'), ('V1'), ('F7A+'), ('F5'), ('5.13c'), ('5.10a');

-- b-tree operators also allow ordering queries
SELECT grade from grades_unique ORDER BY grade;

-- the types can be gotten by calling the grade_type function
SELECT grade, grade_type(grade) FROM grades_unique;

-- grade[] should work
CREATE TABLE grades_array(grade grade[]);
INSERT INTO grades_array VALUES (ARRAY['V5'::grade, 'F7A+'::grade]);
SELECT * FROM grades_array;
