import React from 'react';
import { Link } from 'gatsby';

const AuthorCard = ({ authorKey, author }) => {
  if (!author) return null;

  return (
    <div className="author">
      {author.image && (
        <img src={author.image} alt={author.name} />
      )}
      <span>
        <Link to={`/authors/${authorKey}`}>{author.name}</Link>{' '}
        <span dangerouslySetInnerHTML={{ __html: author.blurb }} />
        <br />
        <AuthorSocial author={author} />
      </span>
    </div>
  );
};

const AuthorSocial = ({ author }) => (
  <span>
    {author.twitter && (
      <a href={`https://twitter.com/${author.twitter}`}>
        <svg className="svg-icon twitter">
          <use xlinkHref="/img/social-icons.svg#twitter"></use>
        </svg>
      </a>
    )}
    {author.linkedin && (
      <a href={author.linkedin}>
        <svg className="svg-icon linkedin">
          <use xlinkHref="/img/social-icons.svg#linkedin"></use>
        </svg>
      </a>
    )}
    {author.github && (
      <a href={`https://github.com/${author.github}`}>
        <svg className="svg-icon github">
          <use xlinkHref="/img/social-icons.svg#github"></use>
        </svg>
      </a>
    )}
  </span>
);

export default AuthorCard;
