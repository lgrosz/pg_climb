CREATE EXTENSION pg_climb;
 
SELECT EXISTS (
    SELECT 1 FROM pg_type WHERE typname = 'grade'
);

SELECT grade_out('V5'::grade);

-- invalid grade because only one type can be specified
CREATE TABLE grades_1(grade grade(verm,font));

-- invalid because "nope" isn't a grade type
CREATE TABLE grades_2(grade grade(nope));

-- any type of grade can be inserted into a column with no typmod
CREATE TABLE grades_3(grade grade);
INSERT INTO grades_3 VALUES ('V5'::grade), ('F7A+'::grade);

-- only values of the given typmod can be inserted when typmod is specified
CREATE TABLE grades_4(grade grade(verm));
INSERT INTO grades_4 VALUES ('V5'::grade);
INSERT INTO grades_4 VALUES ('F7A+'::grade);

