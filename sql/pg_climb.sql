CREATE EXTENSION pg_climb version '0.1';
 
SELECT EXISTS (
    SELECT 1 FROM pg_type WHERE typname = 'grade'
);

SELECT grade_out('V5'::grade);
