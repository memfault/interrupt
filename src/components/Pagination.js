import React from 'react';
import { Link } from 'gatsby';

const TRAIL = 2;

const Pagination = ({ currentPage, numPages }) => {
  if (numPages <= 1) return null;

  const prevPath =
    currentPage === 2 ? '/blog' : `/blog/page${currentPage - 1}`;
  const nextPath = `/blog/page${currentPage + 1}`;
  const pageStart = Math.max(1, currentPage - TRAIL);
  const pageEnd = Math.min(numPages, currentPage + TRAIL);

  const getPath = page => (page === 1 ? '/blog' : `/blog/page${page}`);

  return (
    <div className="pagination d-none d-lg-block">
      {currentPage > 1 ? (
        <Link to={prevPath} className="page-item" title="Previous page">
          &laquo;
        </Link>
      ) : (
        <span className="page-item">&laquo;</span>
      )}

      {pageStart > 1 && (
        <>
          <Link to="/blog" className="page-item" title="First page">
            1
          </Link>
          ...
        </>
      )}

      {Array.from({ length: pageEnd - pageStart + 1 }, (_, i) => {
        const page = pageStart + i;
        if (page === currentPage) {
          return (
            <span key={page} className="page-item">
              {page}
            </span>
          );
        }
        return (
          <Link key={page} to={getPath(page)} className="page-item">
            {page}
          </Link>
        );
      })}

      {pageEnd < numPages && (
        <>
          ...
          <Link to={getPath(numPages)} className="page-item" title="Last page">
            {numPages}
          </Link>
        </>
      )}

      {currentPage < numPages ? (
        <Link to={nextPath} className="page-item" title="Next page">
          &raquo;
        </Link>
      ) : (
        <span className="page-item">&raquo;</span>
      )}
    </div>
  );
};

export default Pagination;
