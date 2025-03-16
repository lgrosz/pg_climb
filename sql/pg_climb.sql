CREATE EXTENSION pg_climb version '0.1';
 
SELECT EXISTS (
    SELECT 1 FROM pg_type WHERE typname = 'grade'
);

SELECT grade_out('V5'::grade);

-- invalid grade because only one type can be specified
CREATE TABLE grades_1(grade grade(verm,font));

-- invalid because "nope" isn't a grade type
CREATE TABLE grades_2(grade grade(nope));

